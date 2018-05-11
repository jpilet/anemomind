if (WITH_SAILROOT)

  add_library(root_sailroot SHARED sailroot.cpp)
  target_link_libraries(root_sailroot
      ${WHOLE_ARCHIVE} anemobox_Dispatcher ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} anemobox_SimulateBox ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} calib_Calibrator ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} common_ArgMap ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} common_DOMUtils ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} common_Env ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} common_PathBuilder ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} common_ScopedLog ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} common_logging ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} device_ChunkFile ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} logimport_LogLoader ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} math_hmm_StateAssign ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} nautical_BoatLogProcessor ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} nautical_BoatSpecificHacks ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} nautical_TargetSpeed ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} nautical_downsamplegps ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} nautical_grammars_TreeExplorer ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} nautical_grammars_WindOrientedGrammar ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} nautical_tiles_TileUtils ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} plot_gnuplot ${NOWHOLE_ARCHIVE}
      ${WHOLE_ARCHIVE} tiles_ChartTiles ${NOWHOLE_ARCHIVE}
      )

  file(GENERATE
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sailroot.C
    CONTENT 
  "void sailroot() {
    gSystem->Load(\"$<TARGET_LINKER_FILE:root_sailroot>\");
    gROOT->ProcessLine(\".I ${CMAKE_SOURCE_DIR}/src\");
    gROOT->SetMacroPath((std::string(gROOT->GetMacroPath()) + \":${CMAKE_CURRENT_SOURCE_DIR}\").c_str());
    gROOT->ProcessLine(\"#include<server/nautical/NavDataset.h>\");
    gROOT->ProcessLine(\"using namespace sail;\");
  }"
  )

  add_custom_target(root COMMAND
    root -l
    -e ".L ${CMAKE_CURRENT_BINARY_DIR}/sailroot.C" -e "'sailroot()'"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
         )
  add_dependencies(root root_sailroot)
endif(WITH_SAILROOT)
