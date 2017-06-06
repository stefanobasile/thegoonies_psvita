cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

set(SHORT_NAME sdltest)
project(${SHORT_NAME})
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "gooniesvita")
set(VITA_TITLEID  "VSDK00013")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(
  common
)

add_executable(${SHORT_NAME}
    src/2DCMC.cpp
    src/auxiliar.cpp
    src/debug.cpp
    src/font_extractor.cpp
    src/geometrics.cpp
    src/GinterfaceBrowser.cpp
    src/GinterfaceButton.cpp
    src/GinterfaceConfirmation.cpp
    src/Ginterface.cpp
    src/GinterfaceMessage.cpp
    src/GinterfaceSButton.cpp
    src/GinterfaceText.cpp
    src/GinterfaceTextInput.cpp
    src/GLTile.cpp
    src/GLTManager.cpp
    src/GMap.cpp
    src/GMapLayer.cpp
    src/GO_bat.cpp
    src/GO_bigrock.cpp
    src/GObject.cpp
    src/GObjectCreator.cpp
    src/GO_bone.cpp
    src/GO_bridge.cpp
    src/GO_bullet.cpp
    src/GO_cagedoor.cpp
    src/GO_character.cpp
    src/GO_closingwall.cpp
    src/GO_coin.cpp
    src/GO_drop.cpp
    src/GO_dropgenerator.cpp
    src/GO_enemy.cpp
    src/GO_entrydoor.cpp
    src/GO_exitdoor.cpp
    src/GO_fallingrock.cpp
    src/GO_fallingwater.cpp
    src/GO_flame.cpp
    src/GO_fratelli.cpp
    src/GO_ghost.cpp
    src/GO_item.cpp
    src/GO_jumpingskull.cpp
    src/GO_key.cpp
    src/GO_lava.cpp
    src/GO_musicalnote.cpp
    src/GooniesScript.cpp
    src/GO_pipe_water.cpp
    src/GO_rope.cpp
    src/GO_skeleton.cpp
    src/GO_skull.cpp
    src/GO_skulldoor.cpp
    src/GO_trickyskull.cpp
    src/GO_water.cpp
    src/GO_wateropening.cpp
    src/GO_watersplash.cpp
    src/keyboardstate.cpp
    src/Level.cpp
    src/LevelPack.cpp
    src/main.cpp
    src/MusicManager.cpp
    src/PlacedGLTile.cpp
    src/ranrotb.cpp
    src/SDL_glutaux.cpp
    src/SFXManager.cpp
    src/sound.cpp
    src/SoundManager.cpp
    src/state_credits.cpp
    src/state_editor.cpp
    src/state_endsequence.cpp
    src/state_game.cpp
    src/state_gameover.cpp
    src/state_gamestart.cpp
    src/state_howtoplay.cpp
    src/state_interlevel.cpp
    src/state_konami.cpp
    src/state_mapeditor.cpp
    src/state_msx.cpp
    src/state_splash.cpp
    src/state_titleanimation.cpp
    src/state_title.cpp
    src/Symbol.cpp
    src/TheGooniesApp.cpp
    src/TheGoonies.cpp
    src/Vector.cpp
)

target_link_libraries(${SHORT_NAME}
  SDL2
  SDL2_image
  vita2d
  SceDisplay_stub
  SceCtrl_stub
  SceAudio_stub
  SceSysmodule_stub
  SceGxm_stub
  SceCommonDialog_stub
  m
  unrar
  png
  freetype
  jpeg
  z
  c
)

vita_create_self(${SHORT_NAME}.self ${SHORT_NAME})
vita_create_vpk(${SHORT_NAME}.vpk ${VITA_TITLEID} ${SHORT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)
