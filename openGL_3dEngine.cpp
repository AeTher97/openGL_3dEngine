/*

 OpenGL - Szablon 3D do cwiczen laboratoryjnych
 (C) Micha³ Turek.

*/

#include <windows.h>
#include "GL/glut.h"
#include "include/model3DS.h"
#include "model/Airplane.h"
#include <time.h>
#include <direct.h>
#include <GL/glu.h>
#include <cmath>

using namespace airplane;

//#include <GL/glaux.h>
//#define GLUTCHECKLOOP

// Wymiary okna
int oknoSzerkosc = 800;
int oknoWysokosc = 600;
bool oknoFullScreen = false;
GLint oknoLewe = 1, oknoPrawe = 2;      // id okien stereo

// Opcje projekcji stereo
int stereoTryb = 0;
int stereoRozstawOczu = 5;                // dystans miêdzy oczami
int stereoPunktPatrzenia = 150;            // odleg³oœæ do punktu, na który patrz¹ oczy
bool stereoIDRamki = false;
bool timing100FPS = true;                // flaga polecenia odmierzania czas
// Kamera
int pozycjaMyszyX;                        // na ekranie
int pozycjaMyszyY;
double kameraX;
double kameraY;
double kameraZ;
double kameraPunktY;
double kameraPredkoscPunktY;
double kameraPredkosc;
bool kameraPrzemieszczanie;        // przemieszczanie lub rozgl¹danie
double kameraKat;                // kat patrzenia
double kameraPredkoscObrotu;
#define MIN_DYSTANS 0.5            // minimalny dystans od brzegu obszaru ograniczenia kamery
double obszarKamery = 0;

auto plane = Airplane(ThreeDimension::Vector(0.0f, 0.0f, 0.0f), 0.2);


/** REJESTRATOR PRZESZKOD **/

struct przeszkoda {
    przeszkoda *next;
    double posX1;
    double posZ1;
    double posX2;
    double posZ2;
};
przeszkoda *obszarPrzeszkody = NULL;

void resetKamery() {
    kameraX = 0;
    kameraY = 4;
    kameraZ = 40;
    kameraKat = -0.4;
    kameraPunktY = -15;
    kameraPredkoscPunktY = 0;
    kameraPredkosc = 0;
    kameraPredkoscObrotu = 0;
    kameraPrzemieszczanie = true;
}

void ustalObszar(double X) {
    obszarKamery = X;
}

bool wObszarze(double posX, double posZ) {
    if (posX * posX + posZ * posZ > (obszarKamery - MIN_DYSTANS * 2) * (obszarKamery - MIN_DYSTANS * 2)) return false;
    przeszkoda *pom = obszarPrzeszkody;
    while (pom) {
        if (pom->posX1 < posX &&
            pom->posX2 > posX &&
            pom->posZ1 < posZ &&
            pom->posZ2 > posZ)
            return false;
        pom = pom->next;
    }
    return true;
}

void rejestrujPrzeszkode(double X1, double Z1, double X2, double Z2) {
    przeszkoda *pom = new przeszkoda;
    if (X1 > X2) {
        double tmp = X1;
        X1 = X2;
        X2 = tmp;
    }
    if (Z1 > Z2) {
        double tmp = Z1;
        Z1 = Z2;
        Z2 = tmp;
    }
    pom->posX1 = X1;
    pom->posZ1 = Z1;
    pom->posX2 = X2;
    pom->posZ2 = Z2;
    pom->next = obszarPrzeszkody;
    obszarPrzeszkody = pom;
}

void SzablonPrzyciskMyszyWcisniety(int button, int state, int x, int y) {
    switch (state) {
        case GLUT_UP:
            kameraPredkosc = 0;
            kameraPredkoscObrotu = 0;
            kameraPredkoscPunktY = 0;
            break;
        case GLUT_DOWN:
            pozycjaMyszyX = x;
            pozycjaMyszyY = y;
            if (button == GLUT_LEFT_BUTTON)
                kameraPrzemieszczanie = true;
            else
                kameraPrzemieszczanie = false;
            break;
    }
}

void SzablonRuchKursoraMyszy(int x, int y) {
    kameraPredkoscObrotu = -(pozycjaMyszyX - x) * 0.001;
    if (kameraPrzemieszczanie) {
        kameraPredkosc = (pozycjaMyszyY - y) * 0.02;
        kameraPredkoscPunktY = 0;
    } else {
        kameraPredkoscPunktY = (pozycjaMyszyY - y) * 0.06;
        kameraPredkosc = 0;
    }
}

void SzablonKlawiszKlawiaturyWcisniety(GLubyte key, int x, int y) {
    switch (key) {
        case 27:
            exit(1);
            break;
        case ' ':
            if (stereoTryb != 2) glutFullScreen();
            break;

    }

}

void PrzyciskMyszyWcisniety(int button, int state, int x, int y) {
    SzablonPrzyciskMyszyWcisniety(button, state, x, y); // wywolanie standardowej obslugi zdarzen szablonu
}

void RuchKursoraMyszy(int x, int y) {
    SzablonRuchKursoraMyszy(x, y); // wywolanie standardowej obslugi zdarzen szablonu
}

void resetAirplaneModel();

void KlawiszKlawiaturyWcisniety(GLubyte key, int x, int y) {
    SzablonKlawiszKlawiaturyWcisniety(key, x, y);    // wywolanie standardowej obslugi zdarzen szablonu
    switch (key) {
        case 'e':
            glEnable(GL_FOG);
            break;
        case 'd':
            glDisable(GL_FOG);
            break;
        case 'r':
            resetAirplaneModel();
            break;

            //airplane control buttons
        case '+':
            plane.increaseVelocity();
            break;
        case '-':
            plane.decreaseVelocity();
            break;
        case '7':
            plane.yaw += 15.0 * cos((double) plane.roll / 360 * 6.28);
            plane.pitch -= 15.0 * sin((double) plane.roll / 360 * 6.28);
            break;
        case '9':
            plane.yaw -= 15.0 * cos((double) plane.roll / 360 * 6.28);
            plane.pitch += 15.0 * sin((double) plane.roll / 360 * 6.28);
            break;
        case '8':
            plane.pitch += 15.0 * cos((double) plane.roll / 360 * 6.28);
            plane.yaw += 15.0 * sin((double) plane.roll / 360 * 6.28);
            if (plane.pitch < -360) {
                plane.pitch += 360;
            }
            break;
        case '2':
            plane.pitch -= 15.0 * cos((double) plane.roll / 360 * 6.28);
            plane.yaw -= 15.0 * sin((double) plane.roll / 360 * 6.28);
            if (plane.pitch > 360) {
                plane.pitch -= 360;
            }
            break;
        case '4':
            plane.roll -= 15;
            if (plane.roll < -360) {
                plane.roll += 360;
            }

            break;
        case '6':
            plane.roll += 15;
            if (plane.roll > 360) {
                plane.roll -= 360;
            }
            break;

    }
}

void resetAirplaneModel() {
    plane.reset();
}


void KlawiszSpecjalnyWcisniety(GLint key, int x, int y) {

    switch (key) {
        case GLUT_KEY_LEFT:

            break;
        case GLUT_KEY_RIGHT:

            break;
        case GLUT_KEY_UP:
            kameraY++;
            break;
        case GLUT_KEY_DOWN:
            kameraY--;
            break;
    }
}


/******************************************************/



void windowInit() {
    glClearColor(0.2, 0.2, 1.0, 0.0);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_LIGHTING);
    GLfloat ambient[4] = {0.3, 0.3, 0.3, 1};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

    GLfloat diffuse[4] = {0.9, 0.9, 0.9, 1};
    GLfloat specular[4] = {0.9, 0.9, 0.9, 1};
    GLfloat position[4] = {30, 30, -30, 1};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glEnable(GL_LIGHT0);  // œwiatlo sceny

    /*******************MGLA**************************/

    float fogColor[4] = {0.8f, 0.8f, 0.8f, 0.1f};
    glFogi(GL_FOG_MODE, GL_EXP2); // [GL_EXP, GL_EXP2, GL_LINEAR ]
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.01f);
    glFogf(GL_FOG_START, 0.0f);
    glFogf(GL_FOG_END, 200.0f);
    glEnable(GL_FOG);


}

void rozmiar(int width, int height) {
    if (width == 0) width++;
    if (width == 0) width++;
    if (stereoTryb != 2) {
        oknoSzerkosc = width;   // przy stereo nie mo¿na zmieniaæ rozmiaru
        oknoWysokosc = height;
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, oknoSzerkosc, oknoWysokosc + 24);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat) oknoSzerkosc / (GLfloat) oknoWysokosc, 1.0f, 10000.0f);
    glMatrixMode(GL_MODELVIEW);

}

void rozmiarLewe(int width, int height) {
    glutSetWindow(oknoLewe);
    rozmiar(width, height);
}

void rozmiarPrawe(int width, int height) {
    glutSetWindow(oknoPrawe);
    rozmiar(width, height);
}
struct model_w_skladzie {
    char *filename;
    model3DS *model;
    struct model_w_skladzie *wsk;
};
struct model_w_skladzie *sklad_modeli = NULL;

void drawCesnaModel();

void dodajModel(model3DS *_model, char *file_name) {
    struct model_w_skladzie *tmp;
    tmp = (struct model_w_skladzie *) malloc(sizeof(struct model_w_skladzie));
    tmp->filename = (char *) malloc(strlen(file_name) + 1);
    strcpy(tmp->filename, file_name);
    tmp->model = _model;
    tmp->wsk = sklad_modeli;
    sklad_modeli = tmp;
}

model3DS *pobierzModel(char *file_name) {
    struct model_w_skladzie *sklad_tmp = sklad_modeli;
    while (sklad_tmp) {
        if (!_stricmp(sklad_tmp->filename, file_name)) return sklad_tmp->model;
        char file_name_full[_MAX_PATH];
        strcpy(file_name_full, file_name);
        strcat(file_name_full, ".3ds");
        if (!_stricmp(sklad_tmp->filename, file_name_full)) return sklad_tmp->model;

        sklad_tmp = sklad_tmp->wsk;
    }
    return NULL;
}

void rysujModel(char *file_name, int tex_num = -1) {
    model3DS *model_tmp;
    if (model_tmp = pobierzModel(file_name))
        if (tex_num == -1)
            model_tmp->draw();
        else
            model_tmp->draw(tex_num, false);

}

void aktywujSpecjalneRenderowanieModelu(char *file_name, int spec_id = 0) {
    model3DS *model_tmp;
    if (model_tmp = pobierzModel(file_name))
        model_tmp->setSpecialTransform(spec_id);
}

void ladujModele() {
    WIN32_FIND_DATA *fd;
    HANDLE fh;
    model3DS *model_tmp;
    char directory[_MAX_PATH];
    if (_getcwd(directory, _MAX_PATH) == NULL) return;
    strcat(directory, "\\data\\*.3ds");

    fd = (WIN32_FIND_DATA *) malloc(sizeof(WIN32_FIND_DATA));
    fh = FindFirstFile((LPCSTR) directory, fd);
    if (fh != INVALID_HANDLE_VALUE)
        do {
            if (fd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {    // katalogi ignorujemy
                if (FindNextFile(fh, fd)) continue; else break;
            }
            // ladowanie obiektu i dodanie do kontenera
            char filename[_MAX_PATH];
            strcpy(filename, "data\\");
            strcat(filename, fd->cFileName);
            model_tmp = new model3DS(filename, 1, stereoTryb == 2);
            dodajModel(model_tmp, fd->cFileName);
            printf("[3DS] Model '%s' stored\n", fd->cFileName);
        } while (FindNextFile(fh, fd));
}
void draw() {

    glDisable(GL_FOG);

    //glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    glPushMatrix();
    glTranslatef(0, 1, 0);
   // rysujModel("teren"); // malowanie pod³o¿a
    rysujModel("niebo"); // malowanie nieba
    glPopMatrix();



    drawCesnaModel();


}

void drawCesnaModel() {
    glPushMatrix();
    float cosP = cos((90.0 - (double) plane.pitch) / 360 * 6.28);
    float sinP = sin((90.0 - (double) plane.pitch) / 360 * 6.28);

    float cosY = cos((double) plane.yaw / 360 * 6.28);
    float sinY = sin((double) plane.yaw / 360 * 6.28);

    plane.position.y = plane.position.y - plane.getVelocity() * cosP;
    plane.position.z = plane.position.z + plane.getVelocity() * sinP * cosY;
    plane.position.x = plane.position.x + plane.getVelocity() * sinP * sinY;


    glTranslatef(plane.position.x, plane.position.y, plane.position.z);


    glRotatef(plane.yaw, 0, 1, 0);
    glRotatef(plane.pitch, 1, 0, 0);
    glRotatef(plane.roll, 0, 0, 1);

    rysujModel("cesna_tex");

    glScalef(5, 5, 5);
    glColor3f(255, 255, 255);
    glPopMatrix();
}

void rysujRamke(bool prawa) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Kasowanie ekranu
    glLoadIdentity();
    switch (stereoTryb) {
        case 0: // zwykle mono
            gluLookAt(kameraX, kameraY, kameraZ, kameraX + 100 * sin(kameraKat), 3 + kameraPunktY,
                      kameraZ - 100 * cos(kameraKat), 0, 1, 0); // kamera
            break;
        case 1: // 3D-ready
        case 2: // pelne stereo
        case 3: // anaglyph
            if (prawa) {            //  klatka prawa
                float newKameraX = kameraX - stereoRozstawOczu / 2 * cos(kameraKat);
                float newKameraZ = kameraZ - stereoRozstawOczu / 2 * sin(kameraKat);
                gluLookAt(newKameraX, kameraY, newKameraZ, kameraX + 0.2 + stereoPunktPatrzenia * sin(kameraKat),
                          3 + kameraPunktY, kameraZ - stereoPunktPatrzenia * cos(kameraKat), 0, 1, 0); // kamera
            } else {                // klatka lewa
                float newKameraX = kameraX + stereoRozstawOczu / 2 * cos(kameraKat);
                float newKameraZ = kameraZ + stereoRozstawOczu / 2 * sin(kameraKat);
                gluLookAt(newKameraX, kameraY, newKameraZ, kameraX + 0.2 + stereoPunktPatrzenia * sin(kameraKat),
                          3 + kameraPunktY, kameraZ - stereoPunktPatrzenia * cos(kameraKat), 0, 1, 0); // kamera
            }
            break;
    } //case


    draw();

    glFlush();
    glPopMatrix();
}


void rysuj() {
    switch (stereoTryb) {
        case 0: // mono
            rysujRamke(false);
            glutSwapBuffers();
            break;
        case 1: // 3D-ready
            stereoIDRamki = !stereoIDRamki;
            rysujRamke(stereoIDRamki);
            glutSwapBuffers();
            break;
        case 2: // pelne stereo
            glutSetWindow(oknoLewe);
            rysujRamke(false);
            glutSetWindow(oknoPrawe);
            rysujRamke(true);
            glutSetWindow(oknoLewe);
            glutSwapBuffers();            // Wyslanie na ekran (L+P) synchronizowane
            glutSetWindow(oknoPrawe);
            glutSwapBuffers();
            break;
        case 3: // anaglyph
            glColorMask(true, false, false, false);
            rysujRamke(true);
            glClear(GL_DEPTH_BUFFER_BIT);
            glColorMask(false, true, true, false);
            rysujRamke(false);
            glColorMask(true, true, true, true);
            glutSwapBuffers();
    }
}

void timer() {
    double kameraXTmp = kameraX + kameraPredkosc * sin(kameraKat);
    double kameraZTmp = kameraZ - kameraPredkosc * cos(kameraKat);
    kameraKat = kameraKat + kameraPredkoscObrotu;
    kameraPunktY = kameraPunktY + kameraPredkoscPunktY;
    if (wObszarze(kameraXTmp, kameraZTmp)) {
        kameraX = kameraXTmp;
        kameraZ = kameraZTmp;
    } else
        kameraPredkosc = 0;
    rysuj();
}

void syncTimer(int ID) {
    timer();
    glutTimerFunc(1, syncTimer, 10);
}

void config() {
    oknoFullScreen = false;



    // KONFIGURACJA	KAMERY


    ustalObszar(500);                        // promien obszaru (kola), po jakim kamera mo¿e sie poruszaæ

    // wymiary terenu: (-200,-200) - (200,200)

    // Funkcja rejestruj¹ca przeszkody - kolejne parametry to X1, Z1, X2, Z2 prostok¹ta, który jest dodatkowo zabroniony dla kamery

    // blokady terenu (sciany)
    rejestrujPrzeszkode(-200, -200, -147, 200);    // lewy brzeg
    rejestrujPrzeszkode(132, -200, 200, 200);        // prawy  brzeg
    rejestrujPrzeszkode(-200, -130, 200, -200);    // przedni brzeg
    rejestrujPrzeszkode(-200, 157, 200, 200);        // tylny brzeg

    rejestrujPrzeszkode(-101, -90, -26, 200);        // lewa wyspa
    rejestrujPrzeszkode(13, -200, 90, 132);        // prawa wyspa
    // Funkcjê mo¿na wywo³ywac wielokrotnie dla ró¿nych obszarów


}

int main(int argc, char **argv) {
    config();
    if (argc > 1 && argv[1][0] == '-' &&
        argv[1][1] == 's')    // poprawki w konfiguracji na podstawie parametró (te maj¹ pierwszeñstwo)
    {
        stereoTryb = 2;
        oknoSzerkosc = 800;
        oknoWysokosc = 600;
    }
    glutInit(&argc, argv);        // INIT - wszystkie funkcje obs³ugi okna i przetwzrzania zdarzeñ realizuje GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    if (stereoTryb == 2) {
        glutInitWindowSize(oknoSzerkosc - 8, oknoWysokosc);
        glutInitWindowPosition(0, 0);
        oknoLewe = glutCreateWindow("Lewe");  // ==1
        HWND hwnd = FindWindow(NULL, "Lewe");
        SetWindowLong(hwnd, GWL_STYLE, WS_BORDER | WS_MAXIMIZE);
        glutSetWindow(oknoLewe);
        windowInit();
        glutReshapeFunc(rozmiarLewe);                        // def. obs³ugi zdarzenia resize (GLUT)
        glutKeyboardFunc(KlawiszKlawiaturyWcisniety);        // def. obs³ugi klawiatury
        glutSpecialFunc(KlawiszSpecjalnyWcisniety);        // def. obs³ugi klawiatury (klawisze specjalne)
        glutMouseFunc(PrzyciskMyszyWcisniety);            // def. obs³ugi zdarzenia przycisku myszy (GLUT)
        glutMotionFunc(RuchKursoraMyszy);                    // def. obs³ugi zdarzenia ruchu myszy (GLUT)
        glutDisplayFunc(rysuj);                                // def. funkcji rysuj¹cej
        glutInitWindowSize(oknoSzerkosc - 8, oknoWysokosc);
        glutInitWindowPosition(oknoSzerkosc + 4, 0);
        oknoPrawe = glutCreateWindow("Prawe"); // ==2
        glutSetWindow(oknoPrawe);
        windowInit();
        hwnd = FindWindow(NULL, "Prawe");
        SetWindowLong(hwnd, GWL_STYLE, WS_BORDER | WS_MAXIMIZE);
        glutReshapeFunc(rozmiarPrawe);                        // def. obs³ugi zdarzenia resize (GLUT)
        glutKeyboardFunc(KlawiszKlawiaturyWcisniety);        // def. obs³ugi klawiatury
        glutSpecialFunc(KlawiszSpecjalnyWcisniety);        // def. obs³ugi klawiatury (klawisze specjalne)
        glutMouseFunc(PrzyciskMyszyWcisniety);            // def. obs³ugi zdarzenia przycisku myszy (GLUT)
        glutMotionFunc(RuchKursoraMyszy);                    // def. obs³ugi zdarzenia ruchu myszy (GLUT)
        glutDisplayFunc(rysuj);                                // def. funkcji rysuj¹cej
    } else {  // jedno okno
        glutInitWindowSize(oknoSzerkosc, oknoWysokosc);
        glutInitWindowPosition(0, 0);
        oknoLewe = glutCreateWindow("Szablon");
        windowInit();
        glutReshapeFunc(rozmiar);                        // def. obs³ugi zdarzenia resize (GLUT)
        glutKeyboardFunc(KlawiszKlawiaturyWcisniety);        // def. obs³ugi klawiatury
        glutSpecialFunc(KlawiszSpecjalnyWcisniety);        // def. obs³ugi klawiatury (klawisze specjalne)
        glutMouseFunc(PrzyciskMyszyWcisniety);            // def. obs³ugi zdarzenia przycisku myszy (GLUT)
        glutMotionFunc(RuchKursoraMyszy);                    // def. obs³ugi zdarzenia ruchu myszy (GLUT)
        glutDisplayFunc(rysuj);                                // def. funkcji rysuj¹cej
    }
    if (stereoTryb == 1 || !timing100FPS)
        glutIdleFunc(timer);
    else
        glutTimerFunc(10, syncTimer, 10);
    resetKamery();
    ladujModele();
    aktywujSpecjalneRenderowanieModelu("woda", 1);
    aktywujSpecjalneRenderowanieModelu("most", 2);
    if (oknoFullScreen && stereoTryb != 2) glutFullScreen();
    glutMainLoop();
    return (0);
}