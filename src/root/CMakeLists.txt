file(GENERATE
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sailroot.C
  CONTENT 
"void sailroot() {
  gSystem->Load(\"$<TARGET_LINKER_FILE:nautical_sailroot>\");
  gROOT->ProcessLine(\".I ${CMAKE_SOURCE_DIR}/src\");
  gROOT->SetMacroPath((std::string(gROOT->GetMacroPath()) + \":${CMAKE_CURRENT_SOURCE_DIR}\").c_str());
  gROOT->ProcessLine(\"#include<server/nautical/NavDataset.h>\");
  gROOT->ProcessLine(\"using namespace sail;\");
}"
)

add_custom_target(root COMMAND
  root -l
  -e ".L ${CMAKE_CURRENT_BINARY_DIR}/sailroot.C" -e "'sailroot()'"
       )
add_dependencies(root nautical_sailroot)
