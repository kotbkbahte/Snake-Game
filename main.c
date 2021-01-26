#include <windows.h>
#include <gl/gl.h>



LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

#define mapW 16
#define mapH 16
#define blue_c 0.0f, 0.282f, 0.663f

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

BOOL isKeyPressed[] = {FALSE, FALSE, FALSE, FALSE};
BOOL isGameStart = TRUE;

int snTailTexture, snHeadTexture, snBodyTexture, snCornerTexture;
int btnLightTexture, btnDarkTexture, btnShadowTexture;
int textPlay, textQuit;



typedef struct STile
{
    char tileType;
    float r;
    float g;
    float b;
    BOOL isHereSnake;
} TTile;

typedef struct SColor
{
    float r, g, b;

} TColor, *PColor;


typedef struct SSnakeNode
{
    struct SSnakeNode *next;
    int x, y;
    char nextPos;
    TColor color;
} TSnakeNode, *PSnakeNode;

typedef struct SSnake2
{
    TSnakeNode *head;
    TSnakeNode *tail;
    int dir;
    int move;
    int length;
} TSnake;


typedef struct SButton
{
    char *name;
    int id;
    float vert[8];
    int height, width;
    BOOL hover;

} TButton, *PButton;

typedef struct SText
{
    int x, y, height, width, texture;
    float vert[8];
} TText;

int btnCnt = 2;
int txtCnt = 2;
TButton *pbtn;
TText   *ptxt;



void buttonInit(TButton *btn, char *name, int id, int x, int y, int h, int w)
{
    btn->name = name;
    btn->id   = id;
    btn->height= h;
    btn->width = w;

    int dh = h / 2;
    int dw = w / 2;

    btn->vert[0] = x - dw;   btn->vert[1] = y - dh;
    btn->vert[2] = x + dw;   btn->vert[3] = y - dh;
    btn->vert[4] = x + dw;   btn->vert[5] = y + dh;
    btn->vert[6] = x - dw;   btn->vert[7] = y + dh;
    btn->hover= FALSE;
}


void textInit(TText *txt, int x, int y, int target)
{
    int w, h;

    glBindTexture(GL_TEXTURE_2D, textPlay);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, NULL, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, NULL, GL_TEXTURE_HEIGHT, &h);

    txt->height = h;
    txt->width  = w;
    txt->x      = x;
    txt->y      = y;
    txt->texture = target;

    int dh = h / 2;
    int dw = w / 2;

    txt->vert[0] = x - dw;   txt->vert[1] = y - dh;
    txt->vert[2] = x + dw;   txt->vert[3] = y - dh;
    txt->vert[4] = x + dw;   txt->vert[5] = y + dh;
    txt->vert[6] = x - dw;   txt->vert[7] = y + dh;


}

void textsInit()
{
    ptxt = calloc(txtCnt, sizeof(TText));
    textInit(ptxt,     256, 256 - 48, textPlay);
    textInit(ptxt + 1, 256, 256 + 48, textQuit);


}




void buttonsInit()
{
    pbtn = calloc(btnCnt, sizeof(TButton));
    buttonInit(pbtn, "start", 1, 256, 256 - 48, 64, 288);
    buttonInit(pbtn + 1, "quit", 2, 256, 256 + 48, 64, 288);
}

int width, height;


BOOL CoordsInButton(int x, int y, TButton *btn)
{
    return (x > btn->vert[0]) && (x < btn->vert[4]) &&
           (y > btn->vert[1]) && (y < btn->vert[5]);
}




void createFood();

TTile map[mapW][mapH];
TColor getRandomColor()
{
    TColor cl;
    cl.r = rand() / 32767.0;
    cl.g = rand() / 32767.0;
    cl.b = rand() / 32767.0;
    return cl;
}

PColor createColor(float r, float g, float b)
{
    PColor cl = malloc(sizeof( TColor ));
    cl->r = r;
    cl->g = g;
    cl->b = b;
    return cl;
}


float textureD[] = {0,1, 0,0, 1,0, 1,1};
float textureS[] = {0,0, 1,0, 1,1, 0,1};
float textureA[] = {1,0, 1,1, 0,1, 0,0};
float textureW[] = {1,1, 0,1, 0,0, 1,0};

float *texturesWSAD[] = {textureA, textureW, textureD, textureS};
                      //0         1         2         3

float *texturesUV[] = {textureW, textureS, textureA, textureD};


int snTextures[2];


float UV0[] = {};
float UV1[] = {1,1, 0,1, 0,0, 1,0}; // ^
                                    // |

float UV2[] = {0,1, 0,0, 1,0, 1,1}; // ->

float UV3[] = {0,0, 1,0, 1,1, 0,1}; // |
                                    // V

float UV4[] = {1,0, 1,1, 0,1, 0,0}; // <-



float UV5[] = {1,1, 0,1, 0,0, 1,0}; // ^
                                    // |_

float UV6[] = {0,1, 0,0, 1,0, 1,1}; // ->
                                    // |

float UV7[] = {0,0, 1,0, 1,1, 0,1}; // -|
                                      //  V

float UV8[] = {1,0, 1,1, 0,1, 0,0};   //   |
                                      //  <-

float UV9[] = {1,0, 0,0, 0,1, 1,1};   //   |-
                                      //   V

float UV10[] = {0,0, 0,1, 1,1, 1,0}; //  <-
                                     //   |

float UV11[] = {0,1, 1,1, 1,0, 0,0}; //  ^
                                     // _|

float UV12[] = {1,1, 1,0, 0,0, 0,1}; //  |
                                     //  ->
float *UVS[] = {UV0, UV1, UV2, UV3, UV4, UV5, UV6, UV7, UV8, UV9, UV10, UV11, UV12};
/*
int posToUV[4][4][2] = {
                         { {2, 4}, {3, 10}, {0, 0}, {3, 8} },
                         { {3, 5}, {2, 1}, {3, 11}, {0, 0} },
                         { {0, 0}, {3, 6}, {2, 2}, {3, 12} },
                         { {3, 9}, {0, 0}, {3, 7}, {2, 3} }
                       };
*/
int posToUV[4][4][2] = {
                         { {0, 4}, {1, 10}, {0, 0}, {1, 8} },
                         { {1, 5}, {0, 1}, {1, 11}, {0, 0} },
                         { {0, 0}, {1, 6}, {0, 2}, {1, 12} },
                         { {1, 9}, {0, 0}, {1, 7}, {0, 3} }
                       };

                       // 2 = body
                       // 3 = corner
float vertex[] = {0,0,0,  1,0,0,  1,1,0,  0,1,0};

float Apple[] = {0,0,0, 0.5,0,0, 0.5,0.5,0, 0,0.5,0};


int getCharIndex(char c)
{
    int i;
    switch (c)
    {
        case 'a':
            return 0;
        case 'w':
            return 1;
        case 'd':
            return 2;
        case 's':
            return 3;
    }

}


void loadTextureFromFile(char *file_name, int *target)
{
    int width, height, cnt;
    unsigned char *data = stbi_load(file_name, &width, &height, &cnt, 0);
    glGenTextures(1, target);
    glBindTexture(GL_TEXTURE_2D, *target);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                                    0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
}

void loadTextures()
{
    loadTextureFromFile("textures/SnakeHead.png", &snHeadTexture);
    loadTextureFromFile("textures/SnakeBody.png", &snBodyTexture);
    loadTextureFromFile("textures/SnakeCorner.png", &snCornerTexture);
    loadTextureFromFile("textures/SnakeTail.png", &snTailTexture);
    loadTextureFromFile("textures/ButtonDark.png", &btnDarkTexture);
    loadTextureFromFile("textures/ButtonLight.png", &btnLightTexture);
    loadTextureFromFile("textures/ButtonShadow.png", &btnShadowTexture);

    loadTextureFromFile("textures/Play.png", &textPlay);


    loadTextureFromFile("textures/Quit.png", &textQuit);


    snTextures[0] = snBodyTexture;
    snTextures[1] = snCornerTexture;
}



PSnakeNode snakeNodeInit(PSnakeNode pr, int x, int y)
{
    PSnakeNode sn = malloc(sizeof(TSnakeNode));
    sn->x = x;
    sn->y = y;
    sn->nextPos = 'w';
    if (pr!=NULL)
        pr->next = sn;
    sn->next = NULL;
    sn->color = getRandomColor();
    return sn;
}

void snakeInit(TSnake *sn)
{
    sn->length = 3;
    sn->dir    = 'w';
    PSnakeNode s1, s2, s3;

    s1 = snakeNodeInit(NULL, 3, 1);
    s2 = snakeNodeInit(s1, 3, 2);
    s3 = snakeNodeInit(s2, 3, 3 );

    sn->tail = s1;
    sn->head = s3;
}

int increaseCoord(int c)
{
    if (c == mapH - 1)
    {
        return 0;
    }
    return ++c;
}

int decreaseCoord(int c)
{
    if (c == 0)
    {
        return mapH-1;
    }
    return --c;
}

void updateBody(TSnake *sn)
{
    TSnakeNode *temp;
    temp = sn->tail->next;

    sn->tail->next = NULL;
    sn->head->next = sn->tail;
    sn->head->nextPos = sn->dir;

    sn->head = sn->tail;
    sn->tail = temp;

}

void snakeIncreaseLength(TSnake *sn, int x, int y)
{
    PSnakeNode ns;
    sn->length++;
    ns = snakeNodeInit(sn->head, x, y);
    sn->head->next = ns;
    sn->head->nextPos = sn->dir;

    sn->head = ns;
    map[x][y].tileType = 'e';
    createFood();
}


void snakeMove(TSnake *sn)
{

    int y = sn->head->y;
    int x = sn->head->x;
    switch (sn->dir)
    {
        case 'w':
            x = x;
            y = increaseCoord(y);
            break;
        case 's':
            x = x;
            y = decreaseCoord(y);
            break;
        case 'd':
            x = increaseCoord(x);
            y = y;
            break;

        case 'a':
            x = decreaseCoord(x);
            y = y;
            break;
    }

    if (map[x][y].tileType == 'f')
    {
        map[x][y].isHereSnake = TRUE;
        snakeIncreaseLength(sn, x, y);
        return;
    }

    map[sn->tail->x][sn->tail->y].isHereSnake = FALSE;

    sn->tail->x = x;
    sn->tail->y = y;

    updateBody(sn);
    map[x][y].isHereSnake = TRUE;


}

void createFood()
{
    int x, y;
    do
    {
        x = rand() % mapH;
        y = rand() % mapW;
    }
    while( map[x][y].isHereSnake );

    map[x][y].tileType = 'f';

    return;
}

void mapInit()
{
    for (int j = 0; j < mapH; j++)
        for (int i = 0; i < mapW; i++)
        {
                map[i][j].tileType = 'e';
                map[i][j].r = rand() / 32767.0;
                map[i][j].g = rand() / 32767.0;
                map[i][j].b = rand() / 32767.0;


                if (map[i][j].b > 0.3)
                    map[i][j].b = 0.1;
                if (map[i][j].r > 0.3)
                    map[i][j].r = 0.1;


                if (map[i][j].g < 0.3)
                    map[i][j].g = 0.5;
        }
    createFood();
    createFood();
}



void drawFood(int i, int j)
{
    glPushMatrix();
        glColor3f(1, 0, 0);
        glTranslatef(i+0.25, j+0.25, 0);
        glEnableClientState(GL_VERTEX_ARRAY);

        glVertexPointer(3, GL_FLOAT, 0, Apple);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glDisableClientState(GL_VERTEX_ARRAY);
    glPopMatrix();
}


void drawTile(int i, int j)
{
    glPushMatrix();
        glColor3f(map[i][j].r, map[i][j].g, map[i][j].b);
        glTranslatef(i, j, 0);
        glEnableClientState(GL_VERTEX_ARRAY);

        glVertexPointer(3, GL_FLOAT, 0, vertex);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glDisableClientState(GL_VERTEX_ARRAY);
    glPopMatrix();
}


void drawMap()
{
    glLoadIdentity();
    glScalef(2.0/mapW, 2.0/mapH, 1);
    glTranslatef(-mapW*0.5 , -mapH*0.5, 0);
    for (int j = 0; j < mapH; j++)
        for (int i = 0; i < mapW; i++)
        {
            drawTile(i, j);
            if (map[i][j].tileType == 'f')
            {
                drawFood(i, j);
            }
        }

}

void drawSnakeTail(PSnakeNode sn)
{
    int i = getCharIndex(sn->nextPos);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, snTailTexture);

    glColor3f(1, 1, 1);
    glPushMatrix();
        glTranslatef(sn->x, sn->y ,0);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(3, GL_FLOAT, 0, vertex);
        glTexCoordPointer(2, GL_FLOAT, 0, texturesWSAD[i]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisable(GL_BLEND);
    glPopMatrix();

}

void drawSnakeHead(PSnakeNode sn, TSnake *snake)
{

    int i = getCharIndex(snake->dir);


    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, snHeadTexture);

    glColor3f(1, 1, 1);
    glPushMatrix();
        glTranslatef(sn->x, sn->y ,0);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(3, GL_FLOAT, 0, vertex);
        glTexCoordPointer(2, GL_FLOAT, 0, texturesWSAD[i]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisable(GL_BLEND);
    glPopMatrix();

}



void drawSnakeBody(TSnakeNode *sn, TSnakeNode *pr)
{
    int p1, p2;
    int uv, tx;

    p1 = getCharIndex(sn->nextPos);
    p2 = getCharIndex(pr->nextPos);

    uv = posToUV[p1][p2][1];
    tx = posToUV[p1][p2][0];

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, snTextures[tx]);
    glColor3f(1, 1, 1);
    glPushMatrix();
        glTranslatef(sn->x, sn->y ,0);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(3, GL_FLOAT, 0, vertex);
        glTexCoordPointer(2, GL_FLOAT, 0, *(UVS +uv) );
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisable(GL_BLEND);
    glPopMatrix();



}


void drawSnake(TSnake *sn)
{
    TSnakeNode *t = sn->tail;
    TSnakeNode *temp;

    glLoadIdentity();
    glScalef(2.0/mapW, 2.0/mapH, 1);
    glTranslatef(-mapW*0.5 , -mapH*0.5, 0);

    drawSnakeTail(t);

    temp = t;
    t = t->next;

    while (1)
    {
        drawSnakeBody(t, temp);
        temp = t;
        t = t->next;
        if (t->next == NULL)
        {
            drawSnakeHead(t, sn);
            return;
        }

    }
    return;
}



void drawText(TText *txt)
{
    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, txt->texture);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);



        glVertexPointer(2, GL_FLOAT, 0, txt->vert);
        glTexCoordPointer(2, GL_FLOAT, 0, UV3  );
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisable(GL_BLEND);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);

}

void drawButton(TButton *btn)
{
    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);

    glBindTexture(GL_TEXTURE_2D, btnShadowTexture);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glPushMatrix();
            glTranslatef(8, 8, 0);
            glVertexPointer(2, GL_FLOAT, 0, btn->vert);
            glTexCoordPointer(2, GL_FLOAT, 0, UV1  );
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glPopMatrix();

    glDisable(GL_BLEND);

    glBindTexture(GL_TEXTURE_2D, (btn->hover) ? btnLightTexture : btnDarkTexture );
        glVertexPointer(2, GL_FLOAT, 0, btn->vert);
        glTexCoordPointer(2, GL_FLOAT, 0, UV1  );
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);






    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
}


void drawMenu()
{
    glPushMatrix();
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1, 1);
        int i;
        for (i = 0; i < btnCnt; i++)
        {
            drawButton(pbtn + i);
        }
        for (i = 0; i < txtCnt; i++)
        {
            drawText(ptxt + i);

        }

    glPopMatrix();
}

void snakeDestroy(TSnake *sn)
{
    TSnakeNode *temp;
    TSnakeNode *last = sn->tail;
    while (last != NULL)
    {
        temp = last;
        last = last->next;
        free(temp);
    }

}


void snakeControl(TSnake *sn)
{
    if (GetKeyState('R') < 0)
    {
        isGameStart = FALSE;
        snakeDestroy(sn);
        snakeInit(sn);
    }


    if ( (GetKeyState('W') < 0) && (!isKeyPressed[0]) ) // W
    {
        if (sn->dir != 's')
        {
            sn->dir = 'w';
            isKeyPressed[0] = TRUE;
            return;

        }
    }
    else if ( (GetKeyState('W') >= 0) )
    {
        isKeyPressed[0] = FALSE;
    }

    if ( (GetKeyState('A') < 0) && (!isKeyPressed[1]) ) // A
    {
        if (sn->dir != 'd')
        {
            sn->dir = 'a';
            isKeyPressed[1] = TRUE;
            return;
        }
    }
    else if ( (GetKeyState('A') >= 0) )
    {
        isKeyPressed[1] = FALSE;
    }

    if ( (GetKeyState('S') < 0) && (!isKeyPressed[2]) ) // S
    {
        if (sn->dir != 'w')
        {
            sn->dir = 's';
            isKeyPressed[2] = TRUE;
            return;
        }
    }
    else if ( (GetKeyState('S') >= 0) )
    {
        isKeyPressed[2] = FALSE;
    }

    if ( (GetKeyState('D') < 0) && (!isKeyPressed[3]) ) // D
    {
        if (sn->dir != 'a')
        {
            sn->dir = 'd';
            isKeyPressed[3] = TRUE;
            return;
        }
    }
    else if ( (GetKeyState('D') >= 0) )
    {
        isKeyPressed[3] = FALSE;
    }
    return;
}



void runGameLoop(TSnake *snake)
{
    static long int t1, t2, dt;
    static struct timeval tv;
    gettimeofday(&tv, NULL);
    t1 = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // in milliseconds

    drawMap();
    snakeControl(snake);
    snakeMove(snake);

    drawSnake(snake);


    gettimeofday(&tv, NULL);
    t2 = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // in milliseconds

    dt = t2 - t1;

    if (dt < 80)
    {
        Sleep(80 - dt);
    }
}


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;
    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = (HICON) LoadImage( // returns a HANDLE so we have to cast to HICON
                  NULL,             // hInstance must be NULL when loading from a file
                  "textures/icon.ico",   // the icon file name
                  IMAGE_ICON,       // specifies that the file is an icon
                  0,                // width of the image (we'll specify default later on)
                  0,                // height of the image
                  LR_LOADFROMFILE|  // we want to load a file (as opposed to a resource)
                  LR_DEFAULTSIZE|   // default metrics based on the type (IMAGE_ICON, 32x32)
                  LR_SHARED         // let the system release the handle when it's no longer used
    );
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = wcex.hIcon;

    if (!RegisterClassEx(&wcex))
        return 0;

    RECT wr = {0, 0, 512, 512};
    AdjustWindowRect(&wr, WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "Snake!",
                          WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
                          0,
                          0,
                          wr.right - wr.left,
                          wr.bottom - wr.top,
                          NULL,
                          NULL,
                          NULL,
                          NULL);


    EnableOpenGL(hwnd, &hDC, &hRC);
    ShowWindow(hwnd, nCmdShow);


    TSnake snake2;
    snakeInit(&snake2);

    mapInit();

    buttonsInit();
    loadTextures();
    textsInit();

    isGameStart = FALSE;




    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);


            if (isGameStart)
            {
                runGameLoop(&snake2);


            }
            else
            {
                drawMap();
                drawMenu();
            }


            SwapBuffers(hDC);

        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;


        case WM_LBUTTONDOWN:
            if (isGameStart) return;
            for (int i = 0; i < btnCnt; i++)
                 if (CoordsInButton(LOWORD(lParam), HIWORD(lParam), pbtn + i))
                    {
                        if ((pbtn + i)->id == 2)
                            PostQuitMessage(0);
                        else if ((pbtn + i)->id == 1)
                            isGameStart = TRUE;
                    }
        break;


        case WM_MOUSEMOVE:
            if (isGameStart) return;
            for (int i = 0; i < btnCnt; i++)
                (pbtn + i)->hover = CoordsInButton(LOWORD(lParam), HIWORD(lParam), (pbtn + i));
        break;

        case WM_SIZE:

            width = LOWORD(lParam);
            height = HIWORD(lParam);
            /*
            glViewport(0, 0, width, height);
            glLoadIdentity();
            float k = width / (float)height;
            glOrtho(-k, k, -1,1, -1,1);
            */
        break;


        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                break;
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

