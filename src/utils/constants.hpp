#pragma once

#include <string_view>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include<complex>

using json = nlohmann::json;

// NETWORK INTERFACES NAMES
constexpr std::string_view INFINIBAND = "ib0";
constexpr std::string_view VLAN120 = "VLAN120";
constexpr std::string_view VLAN117 = "VLAN117";

enum communications {
  no_comm,
  class_comm,
  quantum_comm
};

std::unordered_map<std::string, int> comm_map = {

  {"no_comm", no_comm},
  {"class_comm", class_comm},
  {"quantum_comm", quantum_comm}


};

using CunqaInstructions = std::vector<json>; 
using CunqaStateVector = std::vector<std::complex<double>>;
struct MeasurementOutput {
  CunqaStateVector statevector;
  int measure;
};




enum GATES {
    MEASURE,
    ID,
    X,
    Y,
    Z,
    H,
    SX,
    RX,
    RY,
    RZ,
    CX,
    CY,
    CZ,
    ECR
};

std::unordered_map<std::string, int> GATE_NAMES = {
    // MEASURE
    {"measure", MEASURE},

    // ONE GATE NO PARAM
    {"id", ID},
    {"x", X},
    {"y", Y},
    {"z", Z},
    {"h", H},
    {"sx", SX},



    // ONE GATE PARAM
    {"rx", RX},
    {"ry", RY},
    {"rz", RZ},

    // TWO GATE NO PARAM
    {"cx", CX},
    {"cy", CY},
    {"cz", CZ},
    {"ecr", ECR},
};

std::unordered_map<int, std::string> INVERTED_GATE_NAMES = {
    {MEASURE, "measure"},
    {ID, "id"},
    {X, "x"},
    {Y, "y"},
    {Z, "z"},
    {H, "h"},
    {SX, "sx"},
    {RX, "rx"},
    {RY, "ry"},
    {RZ, "rz"},
    {CX, "cx"},
    {CY, "cy"},
    {CZ, "cz"},
    {ECR, "ecr"},
};

const std::vector<std::string> BASIS_GATES_JSON = {
            "u1", "u2", "u3", "u", "p", "r", "rx", "ry", "rz", "id",
            "x", "y", "z", "h", "s", "sdg", "sx", "sxdg", "t", "tdg",
            "swap", "cx", "cy", "cz", "csx", "cp", "cu", "cu1", "cu3",
            "rxx", "ryy", "rzz", "rzx", "ccx", "ccz", "crx", "cry", "crz",
            "cswap"};

const std::vector<std::string> CUNQA_GATES = {
  "measure", "id", "h", "x", "y", "z", "cx", "cy", "cz", "ecr", "c_if_h", "c_if_x","c_if_y","c_if_z","c_if_rx","c_if_ry","c_if_rz","c_if_cx","c_if_cy","c_if_cz", "d_c_if_h", "d_c_if_x","d_c_if_y","d_c_if_z","d_c_if_rx","d_c_if_ry","d_c_if_rz","d_c_if_cx","d_c_if_cy","d_c_if_cz"
};

const std::vector<std::string> COMM_INSTRUCTIONS = {"measure", "id", "h", "x", "y", "z", "rx", "ry", "rz", "cx", "cy", "cz", "ecr", "c_if_h", "c_if_x","c_if_y","c_if_z","c_if_rx","c_if_ry","c_if_rz","c_if_cx","c_if_cy","c_if_cz", "d_c_if_h", "d_c_if_x","d_c_if_y","d_c_if_z","d_c_if_rx","d_c_if_ry","d_c_if_rz","d_c_if_cx","d_c_if_cy","d_c_if_cz", "d_c_if_ecr"};

enum COMM_INSTRUCTIONS_ENUM {
  cunqa_measure,
  cunqa_id,
  cunqa_h,    
  cunqa_x,  
  cunqa_y,
  cunqa_z,
  cunqa_rx,
  cunqa_ry,
  cunqa_rz,
  cunqa_cx,
  cunqa_cy,
  cunqa_cz,
  cunqa_ecr,
  cunqa_c_if_h,
  cunqa_c_if_x,
  cunqa_c_if_y,
  cunqa_c_if_z,
  cunqa_c_if_rx,
  cunqa_c_if_ry,
  cunqa_c_if_rz,
  cunqa_c_if_cx,
  cunqa_c_if_cy,
  cunqa_c_if_cz,
  cunqa_c_if_ecr,
  cunqa_d_c_if_h,
  cunqa_d_c_if_x,
  cunqa_d_c_if_y,
  cunqa_d_c_if_z,
  cunqa_d_c_if_rx,
  cunqa_d_c_if_ry,
  cunqa_d_c_if_rz,
  cunqa_d_c_if_cx,
  cunqa_d_c_if_cy,
  cunqa_d_c_if_cz,
  cunqa_d_c_if_ecr,

};

std::unordered_map<std::string, int> CUNQA_INSTRUCTIONS_MAP = {
  {"measure", cunqa_measure},
  {"id", cunqa_id},
  {"h", cunqa_h},
  {"x", cunqa_x},
  {"y", cunqa_y},
  {"z", cunqa_z},
  {"rx", cunqa_rx},
  {"ry", cunqa_ry},
  {"rz", cunqa_rz},
  {"cx", cunqa_cx},
  {"cy", cunqa_cy},
  {"cz", cunqa_cz},
  {"ecr", cunqa_ecr},
  {"c_if_h", cunqa_c_if_h},
  {"c_if_x", cunqa_c_if_x},
  {"c_if_y", cunqa_c_if_y},
  {"c_if_z", cunqa_c_if_z},
  {"c_if_rx", cunqa_c_if_rx},
  {"c_if_ry", cunqa_c_if_ry},
  {"c_if_rz", cunqa_c_if_rz},
  {"c_if_cx", cunqa_c_if_cx},
  {"c_if_cy", cunqa_c_if_cy},
  {"c_if_cz", cunqa_c_if_cz},
  {"c_if_ecr", cunqa_c_if_ecr},
  {"d_c_if_h", cunqa_d_c_if_h},
  {"d_c_if_x", cunqa_d_c_if_x},
  {"d_c_if_y", cunqa_d_c_if_y},
  {"d_c_if_z", cunqa_d_c_if_z},
  {"d_c_if_rx", cunqa_d_c_if_rx},
  {"d_c_if_ry", cunqa_d_c_if_ry},
  {"d_c_if_rz", cunqa_d_c_if_rz},
  {"d_c_if_cx", cunqa_d_c_if_cx},
  {"d_c_if_cy", cunqa_d_c_if_cy},
  {"d_c_if_cz", cunqa_d_c_if_cz},
  {"d_c_if_ecr", cunqa_d_c_if_ecr},
};

std::unordered_map<std::string, std::string> CORRESPONDENCE_D_GATE_MAP = {
  {"d_c_if_h", "h"},
  {"d_c_if_x", "x"},
  {"d_c_if_y", "y"},
  {"d_c_if_z", "z"},
  {"d_c_if_rx", "rx"},
  {"d_c_if_ry", "ry"},
  {"d_c_if_rz", "rz"},
  {"d_c_if_cx", "cx"},
  {"d_c_if_cy", "cy"},
  {"d_c_if_cz", "cz"},
  {"d_c_if_ecr", "ecr"},
};


const std::string cafe = R"(                                                                                                
                                                            ##*%                                                                                                
                                                          ###%###                                                                                               
                                                      *  ###%%%%%                                                                                               
                                                  #%  #%%#%%%%%%%                                                                                               
                                                  *%%%######%%%##%%%%%%#****                                                                                    
                                                  *###############%%%%%%%%##%##                                                                                 
                                                  %%%%##%**#*%######%%%#%%#####%%#                                                                             
                                                  *#%#%#%##%*%**%###%######%####%%%###                                #%%%%%%%%%                                
                                                   +#%##%%####*%%%%###%##*#####%%#%%#%###                          #%%%%%%%%%@%%###                             
                                                     ###%#%%%#*#@%#############%%#%%%%#######                     %%%%#%%%%%%%%%%@%%%                           
                                                     %####%%@@*%@%#%%###%%###%%#%#######*######                  #@%%%%%@%%%%%%%%#%%@%                          
                                                        %#*#@%#%%%%%#%%%##%####*####*#*#***#%#%##               #%%%*%%%%%%%%@@%%%%@%#                          
                                                        %##%%%%%@@%%@%%%%%%####%%#********####*##*%#         ##%%%%%%%%#%%%#%%%@%@@%%%                          
                                                         ##%%%%%%%@@%%%%%%%%%%%%####%#######%#####%#%#       %%%%%%%%%%%%%%%%#%@@%@%@%%                         
                                                          #%%%%%#@%@%%%%%%%%%%%%@%%%###%%#%##%##%*#####*   #%%%@%%%%%%%##%%@%@@@@%%@%%@                         
                                                                 #%%@%%%%%%%%@%@%@@%%%%%%%@%#%%#%###*##%#%%%%%%%%%%###%%%%%@@@@@@@%@@@%                         
                                                                  %%%@@@@@%@@@@@@@@%%%%#%#%%%####%%####%%%@%%%%%##%%%#%%@@%@@%%%%%@@%@%                         
                                                                   %%%%@@%@@@%%@%%%#%%%%%#####%########%@%%@@%%%%%%%%%%@%%%%%%%%%*@@%@%                         
                                                                    %%%%%%%%@%%%%%%%%%%%%##%%%%%%%#####%%%%#%%%%%%%%%%%@@%%%%%#%**@@%@                          
                                                                @@@   %%%%%%%%%%%%%@@@%%%%#%@%%%##%##%####%%%%%%%%%%%%@@%%@@@%%@@@@%@                           
                                                             %%%%%%#+*##%@%%#%%%%%%%%%%%%%%%%%%%%%##%#%####%%@%%%%%%@@%@%%%@%#%@@%@%%%%%%                       
                                                          %#%%%%%*%%%%%+=-*#%%%%@%%##%%%%%%#%%%#%%%%%##%%%#%@%%%%%%%%%%%%%#%*###%%%                             
                                                        ##%%%%%*=====+##%%*++=*%%%%#%#####%#%##%%#%%%#%%%%%%@@@%@%#%%%%%# ##* %%%                               
                                                      %%*%@%%%%%+=++=++=+%%%@*=*+*#%#*#%%%#**####%##%@%%%%#%@@@%%%%%@%%%###                                     
                                                     %#=:======%%%%#=====+*%%%#**+=*###%%%#**#*%%##%@%%%%##@@@@@@%@%## # #                                      
                                                   %%%+=+===--=+==*%##====+=*+**++***%####**####%%%%%%%%#%%@@@@@%%%%##                                          
                                                 ###+======-::..-==++%%*--=#%%#*#**+#+%%%*##%####%%%%%%%%%%%%%%%%%%%%%                                          
                                                %%==-=-==-=------::--=+%%%%%%%   *#*+*+*####%%%%%%%%%%##%%%%%###%%%                                             
                                              %%+=+**=+=-+++++=++=====+=*#%%#        #**+%%%%%%%%%%%%%#%%%%##*#*                                                
                                            +*#*#%%*++++++++++++++*==#%%%%%*         -#*#%%%%%%%%%%%%%%%%#* %*                                                  
                                           *#*-%#%%%%%@@@@@@@@@@@@@%%#@=#%%           #%#%%%@@@@%%%%%%%%#*+                                                     
                                          +**=  #%%%@@@@@@@@@@@@@%@@%%*-#@            #%#%#%%%%%%##%%%%#%                                                       
                                          ++=   #*#%%%####@@@@@@@@@%##%@%%             %%%%@%%#%%%###%                                                          
                                         =++=    *##########%@%@@%%%#%#%#             %%%%%@%@%@%                                                               
                                         ++=-    -=%%##########%###@%%+.              :+%%%%%%%%%:                                                              
                                        :-=:      :-=#%#######=--%#%%                  :-%%%%*+#-                                                               
                                        :-=         ::+##%##=-:##*%                      :.=+**-                                                                
                                       .--            ::+#*==%#*#                          +***.                                                                
                                                  @@     .+#*=                             +**#:                                                                
                                                 @@                                        +###.                                                              
                                                                                           =###****                                                           
                      @@@@@@    @@   @@@@@@@ @@@@@@@  @@@@@@                              :=###=                                                              
                      @@       @@@@  @@      @@           @@                         -+*+=*####-                                                               
                      @@      @@  @@ @@@@@@@ @@@@@@@    @@@                      =+#######%%##**#-                                                              
                      @@      @@  @@ @@@     @@         @@                               %+##  -##:                                                             
                      @@      @@@@@@ @@@     @@                                         -:+++                                                               
                      @@@@@@  @@  @@ @@      @@@@@@@@   @@                                                                                                                                                                                                                                                                   
)";