#ifdef KITSCHY_DEBUG_MEMORY
#include "debug_memorymanager.h"
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "math.h"
#include "string.h"

//#include "GL/gl.h"
//#include "GL/glu.h"
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "List.h"
#include "Symbol.h"
#include "2DCMC.h"
#include "auxiliar.h"
#include "GLTile.h"
#include "2DCMC.h"
#include "keyboardstate.h"
#include "randomc.h"
#include "VirtualController.h"

#include "GLTManager.h"
#include "SoundManager.h"
#include "SFXManager.h"
#include "MusicManager.h"
#include "GObject.h"
#include "GO_enemy.h"
#include "GO_fratelli.h"
#include "GMap.h"
#include "TheGoonies.h"
#include "TheGooniesCtnt.h"
#include "TheGooniesApp.h"
#include "Level.h"
#include "LevelPack.h"

#include "font_extractor.h"

// opengl source from: ftp://oss.sgi.com/projects/ogl-sample/download/
// implementation of gluPerspective and gluLookAt
/*
** Make m an identity matrix
*/
void __gluMakeIdentityd(GLdouble m[16])
{
    m[0+4*0] = 1; m[0+4*1] = 0; m[0+4*2] = 0; m[0+4*3] = 0;
    m[1+4*0] = 0; m[1+4*1] = 1; m[1+4*2] = 0; m[1+4*3] = 0;
    m[2+4*0] = 0; m[2+4*1] = 0; m[2+4*2] = 1; m[2+4*3] = 0;
    m[3+4*0] = 0; m[3+4*1] = 0; m[3+4*2] = 0; m[3+4*3] = 1;
}

void __gluMakeIdentityf(GLfloat m[16])
{
    m[0+4*0] = 1; m[0+4*1] = 0; m[0+4*2] = 0; m[0+4*3] = 0;
    m[1+4*0] = 0; m[1+4*1] = 1; m[1+4*2] = 0; m[1+4*3] = 0;
    m[2+4*0] = 0; m[2+4*1] = 0; m[2+4*2] = 1; m[2+4*3] = 0;
    m[3+4*0] = 0; m[3+4*1] = 0; m[3+4*2] = 0; m[3+4*3] = 1;
}

void GLAPI
gluOrtho2D(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top)
{
    glOrtho(left, right, bottom, top, -1, 1);
}

#define __glPi 3.14159265358979323846

void GLAPI
gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
    GLdouble m[4][4];
    double sine, cotangent, deltaZ;
    double radians = fovy / 2 * __glPi / 180;

    deltaZ = zFar - zNear;
    sine = sin(radians);
    if ((deltaZ == 0) || (sine == 0) || (aspect == 0)) {
    return;
    }
    cotangent = cos(radians) / sine;

    __gluMakeIdentityd(&m[0][0]);
    m[0][0] = cotangent / aspect;
    m[1][1] = cotangent;
    m[2][2] = -(zFar + zNear) / deltaZ;
    m[2][3] = -1;
    m[3][2] = -2 * zNear * zFar / deltaZ;
    m[3][3] = 0;
    glMultMatrixd(&m[0][0]);
}

static void normalize(float v[3])
{
    float r;

    r = sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
    if (r == 0.0) return;

    v[0] /= r;
    v[1] /= r;
    v[2] /= r;
}

static void cross(float v1[3], float v2[3], float result[3])
{
    result[0] = v1[1]*v2[2] - v1[2]*v2[1];
    result[1] = v1[2]*v2[0] - v1[0]*v2[2];
    result[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

void GLAPI
gluLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez, GLdouble centerx,
      GLdouble centery, GLdouble centerz, GLdouble upx, GLdouble upy,
      GLdouble upz)
{
    int i;
    float forward[3], side[3], up[3];
    GLfloat m[4][4];

    forward[0] = centerx - eyex;
    forward[1] = centery - eyey;
    forward[2] = centerz - eyez;

    up[0] = upx;
    up[1] = upy;
    up[2] = upz;

    normalize(forward);

    /* Side = forward x up */
    cross(forward, up, side);
    normalize(side);

    /* Recompute up as: up = side x forward */
    cross(side, forward, up);

    __gluMakeIdentityf(&m[0][0]);
    m[0][0] = side[0];
    m[1][0] = side[1];
    m[2][0] = side[2];

    m[0][1] = up[0];
    m[1][1] = up[1];
    m[2][1] = up[2];

    m[0][2] = -forward[0];
    m[1][2] = -forward[1];
    m[2][2] = -forward[2];

    glMultMatrixf(&m[0][0]);
    glTranslated(-eyex, -eyey, -eyez);
}

// end of opengl source



/* fps counter: */
extern int frames_per_sec;
extern int frames_per_sec_tmp;

extern int init_time;
extern bool show_fps;
extern bool fullscreen;
extern bool screen_shake;
extern bool water_reflection;
extern bool ambient_light;

extern int difficulty;
extern int hiscore;
extern int current_cycle;
extern SDL_Window *screen_sfc;

TheGooniesApp::TheGooniesApp()
{
    m_state = THEGOONIES_STATE_MSX;
    // m_state = THEGOONIES_STATE_EDITOR;
    // m_state = THEGOONIES_STATE_ENDSEQUENCE;

	m_previous_state = m_state;
    m_state_cycle = 0;
    m_title_state = 0;
    m_title_option_selected = 0;
    m_title_waiting_keystroke = false;
    m_time_since_last_key = 0;

    m_skip_intro_screens = false;

    m_titleanimation_state = 0;
    m_titleanimation_cycle = 0;

	m_title_password[0]=0;

    m_interlevel_state = 0;
    m_interlevel_cycle = 0;

    m_game_state = 0;

    m_gameover_state = 0;
    m_endsequence_state = 0;
	m_endsequence_speed = 0;
	m_endsequence_shift = 0;

	m_mouse_x=0;
	m_mouse_y=0;
	m_draw_mouse_status=0;
	m_draw_mouse=0.0f;
	m_mouse_movement_timmer=0;
	m_mouse_button=0;

	m_editor_levelpack = 0;
	m_editor_level =0;

    m_screen_dx = SCREEN_X;
    m_screen_dy = SCREEN_Y;
    fullscreen = true;

	screen_shake = true;
	water_reflection = true;
	ambient_light = true;	
	
    m_GLTM = new GLTManager();

    m_SFXM = new SFXManager();
    m_SFXM->cache("sfx");
//	m_GLTM->cache();
    m_MusicM = new MusicManager();

    // default keyboard config
    m_keys_configuration[GKEY_UP] = SDL_SCANCODE_UP;
    m_keys_configuration[GKEY_RIGHT] = SDL_SCANCODE_RIGHT;
    m_keys_configuration[GKEY_DOWN] = SDL_SCANCODE_DOWN;
    m_keys_configuration[GKEY_LEFT] = SDL_SCANCODE_LEFT;
    m_keys_configuration[GKEY_FIRE] = SDL_SCANCODE_SPACE;
    m_keys_configuration[GKEY_PAUSE] = SDL_SCANCODE_F1;
    m_keys_configuration[GKEY_QUIT] = SDL_SCANCODE_ESCAPE;

    m_sfx_volume = MIX_MAX_VOLUME;
    m_music_volume = 96;
    m_ambient_volume = 96;
    m_game = 0;

    m_vc = new VirtualController();
	m_num_joysticks = SDL_NumJoysticks();
	if (m_num_joysticks > 0) {
		m_joystick = SDL_JoystickOpen(0);
	} else {
		m_joystick = 0;
	}

//    font_extract("font", "graphics/font.png", 10 + 13 + 13 + 10 + 14 + 8, "1234567890abcdefghijklmnopqrstuvwxyzXXXXXXXXXX-./:\"#$%!?:;.,'`[]{|}ñ");
//    font_extract("font_hl", "graphics/font_highlighted.png", 10 + 13 + 13 + 10 + 14 + 8, "1234567890abcdefghijklmnopqrstuvwxyzXXXXXXXXXX-./:\"#$%!?:;.,'`[]{|}ñ");
    font_extract("font", "graphics/font.png", 10 + 13 + 13 + 10 + 14 + 8, "1234567890abcdefghijklmnopqrstuvwxyz-!?:;.,'/\"#%[]{|}ñ");
    font_extract("font_hl", "graphics/font_highlighted.png", 10 + 13 + 13 + 10 + 14 + 8, "1234567890abcdefghijklmnopqrstuvwxyz-!?:;.,'/\"#%[]{|}ñ");
    font_extract("font_hud", "graphics/font_hud.png", 11, "0123456789-");

    m_test_game = 0;
    load_configuration();
	load_hiscores();
}


TheGooniesApp::~TheGooniesApp()
{
    if (m_game != 0) {
        delete m_game;
	    m_game = 0;
    } // if

	if (m_editor_levelpack !=0) {
		delete m_editor_levelpack;
		m_editor_levelpack = 0;
	} // if

    delete m_GLTM;
    delete m_SFXM;
    delete m_MusicM;

    font_release();

	if (m_joystick != 0) {
		SDL_JoystickClose(m_joystick);
		m_joystick=0;
	}
	
    if (m_vc != 0) {
		delete m_vc;
        m_vc = 0;
    }
    save_configuration();
	save_hiscores();
}


void TheGooniesApp::MouseClick(int mousex,int mousey)
{
	m_mouse_click_x.Add(new int(mousex));
	m_mouse_click_y.Add(new int(mousey));
} /* TheGooniesApp::MouseClick */


bool TheGooniesApp::cycle(KEYBOARDSTATE *k)
{
    int old_state = m_state;

#ifdef __DEBUG_MESSAGES

    if (state_cycle == 0) {
        output_debug_message("First Cycle started for state %i...\n", m_state);
    }
#endif

    switch (m_state) {
        case THEGOONIES_STATE_SPLASH:
            m_state = splash_cycle(k);
            break;
        case THEGOONIES_STATE_MSX:
            m_state = msx_cycle(k);
            break;
        case THEGOONIES_STATE_KONAMI:
            m_state = konami_cycle(k);
            break;
        case THEGOONIES_STATE_TITLE:
            m_state = title_cycle(k);
            break;
        case THEGOONIES_STATE_TITLEANIMATION:
            m_state = titleanimation_cycle(k);
            break;
        case THEGOONIES_STATE_GAMESTART:
            m_state = gamestart_cycle(k);
            break;
        case THEGOONIES_STATE_GAME:
            m_state = game_cycle(k);
            break;
        case THEGOONIES_STATE_GAMEOVER:
            m_state = gameover_cycle(k);
            break;
        case THEGOONIES_STATE_ENDSEQUENCE:
            m_state = endsequence_cycle(k);
            break;
        case THEGOONIES_STATE_INTERLEVEL:
            m_state = interlevel_cycle(k);
            break;
        case THEGOONIES_STATE_HOWTOPLAY:
            m_state = howtoplay_cycle(k);
            break;
        case THEGOONIES_STATE_CREDITS:
            m_state = credits_cycle(k);
            break;
        case THEGOONIES_STATE_EDITOR:
            m_state = editor_cycle(k);
            break;
        case THEGOONIES_STATE_MAPEDITOR:
            m_state = mapeditor_cycle(k);
            break;
        default:
            return false;
    }

    if (m_game != 0 && m_game->get_hiscore() > hiscore) {
        set_hiscore(m_game->get_hiscore());
    }

    if (old_state == m_state) {
        m_state_cycle++;
    } else {
        m_state_cycle = 0;

#ifdef __DEBUG_MESSAGES
        output_debug_message("State change: %i -> %i\n", old_state, m_state);
#endif
    }

    m_SFXM->next_cycle();
    m_MusicM->next_cycle();

	m_previous_state = old_state;

    return true;
}

//void perspectiveGL( GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar )
//{
//    const GLdouble pi = 3.1415926535897932384626433832795;
//    GLdouble fW, fH;
//
//    //fH = tan( (fovY / 2) / 180 * pi ) * zNear;
//    fH = tan( fovY / 360 * pi ) * zNear;
//    fW = fH * aspect;
//
//    glFrustum( -fW, fW, -fH, fH, zNear, zFar );
//}
//
////Compat method: gluLookAt deprecated
//void util_compat_gluLookAt(GLfloat eyeX, GLfloat eyeY, GLfloat eyeZ, GLfloat lookAtX, GLfloat lookAtY, GLfloat lookAtZ, GLfloat upX, GLfloat upY, GLfloat upZ) {
//    Vector3f x, y, z;
//    z = Vector3f(eyeX-lookAtX, eyeY-lookAtY, eyeZ-lookAtZ).normalize();
//    y = Vector3f(upX, upY, upZ);
//    x = y ^ z;
//    y = z ^ x;
//    x = x.normalize();
//    y = y.normalize();
//    // mat is given transposed so OpenGL can handle it.
//    Matrix4x4 mat (new GLfloat[16]
//                     {x.getX(), y.getX(),   z.getX(),   0,
//                     x.getY(),  y.getY(),   z.getY(),   0,
//                     x.getZ(),  y.getZ(),   z.getZ(),   0,
//                     -eyeX,     -eyeY,      -eyeZ,      1});
//    glMultMatrixf(mat.getComponents());
//}

void TheGooniesApp::draw()
{
    float lightpos[4] = {0, 0, -1000, 0};
    float tmpls[4] = {1.0F, 1.0F, 1.0F, 1.0};
    float tmpld[4] = {1.0F, 1.0F, 1.0F, 1.0};
    float tmpla[4] = {1.0F, 1.0F, 1.0F, 1.0};
    float ratio;

    m_screen_dx = SCREEN_X;
    m_screen_dy = SCREEN_Y;

#ifdef __DEBUG_MESSAGES

    if (state_cycle == 0) {
        output_debug_message("First Drawing cycle started for state %i...\n", state);
    }
#endif

    /* Enable Lights, etc.: */
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective Calculations
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, tmpla);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, tmpld);
    glLightfv(GL_LIGHT0, GL_SPECULAR, tmpls);

    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT, GL_FILL);

    glClearColor(0, 0, 0, 0.0);
    glViewport(0, 0, SCREEN_X, SCREEN_Y);
    ratio = (float)SCREEN_X / float(SCREEN_Y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, ratio, 1.0, 10240.0);
    gluLookAt(320, 240, PERSPECTIVE_DISTANCE, 320, 240, 0, 0, -1, 0); /* for 640x480 better */

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    switch (m_state) {
        case THEGOONIES_STATE_SPLASH:
            splash_draw();
            break;
        case THEGOONIES_STATE_MSX:
            // FIXME: this is a bit jucky; m_SFXM should be passed in the
            // constructor, not the draw() method
            msx_draw(m_SFXM);
            break;
        case THEGOONIES_STATE_KONAMI:
            konami_draw();
            break;
        case THEGOONIES_STATE_TITLE:
            title_draw();
            break;
        case THEGOONIES_STATE_TITLEANIMATION:
            titleanimation_draw();
            break;
        case THEGOONIES_STATE_GAMESTART:
            gamestart_draw();
            break;
        case THEGOONIES_STATE_GAME:
            game_draw();
            break;
        case THEGOONIES_STATE_GAMEOVER:
            gameover_draw();
            break;
        case THEGOONIES_STATE_ENDSEQUENCE:
            endsequence_draw();
            break;
        case THEGOONIES_STATE_INTERLEVEL:
            interlevel_draw();
            break;
        case THEGOONIES_STATE_HOWTOPLAY:
            howtoplay_draw();
            break;
        case THEGOONIES_STATE_CREDITS:
            credits_draw();
            break;
        case THEGOONIES_STATE_EDITOR:
            editor_draw();
            break;
        case THEGOONIES_STATE_MAPEDITOR:
            mapeditor_draw();
            break;
    }

    show_fps = true;
    if (show_fps) {
        char tmp[80];
        sprintf(tmp, "video mem: %.4gmb - fps: %i", float(GLTile::get_memory_used()) / float(1024 * 1024), frames_per_sec);
        font_print_c(320, 460, 0, 0, 0.5f, "font_hl", tmp, -2);
    }

    glDisable(GL_BLEND);

// MIGRATION
//    SDL_GL_SwapBuffers();
    SDL_GL_SwapWindow(screen_sfc);
//    printf(".");
}


int TheGooniesApp::screen_x(int x)
{
    return ((x * m_screen_dx) / SCREEN_X);
}


int TheGooniesApp::screen_y(int y)
{
    return ((y*m_screen_dy) / SCREEN_Y);
}


void TheGooniesApp::save_configuration(void)
{
    int i;
    FILE *fp;
    char cfg[255];
#ifdef _WIN32

    sprintf(cfg, "goonies.cfg");
#else

    snprintf(cfg, 255, "%s/.goonies.cfg", getenv("HOME"));
#endif //_WIN32

    fp = fopen(cfg, "w");
    if (fullscreen) {
        fprintf(fp, "fullscreen\n");
    } else {
        fprintf(fp, "windowed\n");
    }
    fprintf(fp, "%i %i %i\n" , m_sfx_volume, m_music_volume, m_ambient_volume);
    fprintf(fp, "%i\n", difficulty);

    for (i = 0; i < 7; i++) {
        fprintf(fp, "%i ", m_keys_configuration[i]);
    }
    fprintf(fp, "\n");

	fprintf(fp, "screen_shake_%s\n", (screen_shake ? "on" : "off"));
	fprintf(fp, "water_reflection_%s\n", (water_reflection ? "on" : "off"));
	fprintf(fp, "ambient_light_%s\n", (ambient_light ? "on" : "off"));		

    fclose(fp);
}


void TheGooniesApp::load_configuration(void)
{
    FILE *fp;
    char cfg[255];
#ifdef _WIN32

    sprintf(cfg, "goonies.cfg");
#else

    snprintf(cfg, 255, "%s/.goonies.cfg", getenv("HOME"));
#endif //_WIN32

    fp = fopen(cfg, "r");
    if (fp == 0) {
        save_configuration();
    } else {
        char tmp_s[80];
        int i;

        // fullscreen / windowed
        fscanf(fp, "%s", tmp_s);
        if (strcmp(tmp_s, "fullscreen") == 0) {
            fullscreen = true;
        } else {
            fullscreen = false;
        }
        // volumes
        fscanf(fp, "%i %i %i", &m_sfx_volume, &m_music_volume, &m_ambient_volume);

        // difficulty
        fscanf(fp, "%i", &difficulty);

// MIGRATION FIX
//        // keyboard
//        for (i = 0; i < 7; i++) {
//            fscanf(fp, "%i", &(m_keys_configuration[i]));
//        }
		// screen shake
		fscanf(fp, "%s", tmp_s);
		screen_shake = (strcmp(tmp_s, "screen_shake_on") ? false : true);

		// water reflections
		fscanf(fp, "%s", tmp_s);
		water_reflection = (strcmp(tmp_s, "water_reflection_on") ? false : true);

		fscanf(fp, "%s", tmp_s);
		ambient_light = (strcmp(tmp_s, "ambient_light_on") ? false : true);
		
        fclose(fp);
    }
}


void TheGooniesApp::save_hiscores(void)
{
    FILE *fp;
    char cfg[255];
#ifdef _WIN32

    sprintf(cfg, "goonies.hi");
#else

    snprintf(cfg, 255, "%s/.goonies.hi", getenv("HOME"));
#endif //_WIN32

    fp = fopen(cfg, "w");
	fprintf(fp, "%i\n", hiscore);
    fclose(fp);
}

void TheGooniesApp::load_hiscores(void)
{
    FILE *fp;
    char cfg[255];
#ifdef _WIN32

    sprintf(cfg, "goonies.hi");
#else

    snprintf(cfg, 255, "%s/.goonies.hi", getenv("HOME"));
#endif //_WIN32

    fp = fopen(cfg, "r");
    if (fp == 0) {
        save_hiscores();
    } else {
		hiscore = 0;
		fscanf(fp, "%i", &hiscore);

        fclose(fp);
    }
}
