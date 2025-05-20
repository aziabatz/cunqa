#pragma once

#include <vector>
#include <complex>
#include <unordered_map>

namespace cunqa {
namespace constants{

enum INSTRUCTIONS {
    UNITARY,
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
    CRX,
    CRY,
    CRZ,
    ECR,
    CECR,
    C_IF_X,
    C_IF_Y,
    C_IF_Z,
    C_IF_H,
    C_IF_SX,
    C_IF_RX,
    C_IF_RY,
    C_IF_RZ,
    C_IF_CX,
    C_IF_CY,
    C_IF_CZ,
    C_IF_ECR,
    MEASURE_AND_SEND,
    REMOTE_C_IF_X,
    REMOTE_C_IF_Y,
    REMOTE_C_IF_Z,
    REMOTE_C_IF_H,
    REMOTE_C_IF_SX,
    REMOTE_C_IF_RX,
    REMOTE_C_IF_RY,
    REMOTE_C_IF_RZ,
    REMOTE_C_IF_CX,
    REMOTE_C_IF_CY,
    REMOTE_C_IF_CZ,
    REMOTE_C_IF_ECR,
};

const std::unordered_map<std::string, int> INSTRUCTIONS_MAP = {
    // UNITARY
    {"unitary", UNITARY},

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
    {"crx", CRX},
    {"cry", CRY},
    {"crz", CRZ},
    {"ecr", ECR},

    // TWO GATE NO PARAM
    {"cecr", CECR},

    //CLASSICAL CONTROLLED GATES
    {"c_if_x", C_IF_X},
    {"c_if_y", C_IF_Y},
    {"c_if_z", C_IF_Z},
    {"c_if_h", C_IF_H},
    {"c_if_sx", C_IF_H},
    {"c_if_rx", C_IF_SX},
    {"c_if_ry", C_IF_RY},
    {"c_if_rz", C_IF_RZ},
    {"c_if_cx", C_IF_CX},
    {"c_if_cy", C_IF_CY},
    {"c_if_cz", C_IF_CZ},
    {"c_if_ecr", C_IF_ECR},

    // SEND CLASSICAL QUBIT
    {"measure_and_send", MEASURE_AND_SEND},

    // REMOTE CONTROLLED GATES
    {"remote_c_if_x", REMOTE_C_IF_X},
    {"remote_c_if_y", REMOTE_C_IF_Y},
    {"remote_c_if_z", REMOTE_C_IF_Z},
    {"remote_c_if_h", REMOTE_C_IF_H},
    {"remote_c_if_sx", REMOTE_C_IF_SX},
    {"remote_c_if_rx", REMOTE_C_IF_RX},
    {"remote_c_if_ry", REMOTE_C_IF_RY},
    {"remote_c_if_rz", REMOTE_C_IF_RZ},
    {"remote_c_if_cx", REMOTE_C_IF_CX},
    {"remote_c_if_cy", REMOTE_C_IF_CY},
    {"remote_c_if_cz", REMOTE_C_IF_CZ},
    {"remote_c_if_ecr", REMOTE_C_IF_ECR},
};

const std::unordered_map<int, std::string> INVERTED_GATE_NAMES = {
    {UNITARY, "unitary"},
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
    {CRX, "crx"},
    {CRY, "cry"},
    {CRZ, "crz"},
    {ECR, "ecr"},
};

const std::vector<std::string> BASIS_GATES = {
    "u1", "u2", "u3", "u", "p", "r", "rx", "ry", "rz", "id",
    "x", "y", "z", "h", "s", "sdg", "sx", "sxdg", "t", "tdg",
    "swap", "cx", "cy", "cz", "csx", "cp", "cu", "cu1", "cu3",
    "rxx", "ryy", "rzz", "rzx", "ccx", "ccz", "crx", "cry", "crz",
    "cswap"
};

const std::vector<std::string> BASIS_AND_DISTRIBUTED_GATES = {
    "id", "x", "y", "z", "h", "sx", "cx", "cy", "cz", "ecr", "c_if_x","c_if_y","c_if_z", "c_if_h", "c_if_sx", "c_if_rx","c_if_ry","c_if_rz","c_if_cx","c_if_cy","c_if_cz", "measure_and_send", "remote_c_if_x","remote_c_if_y","remote_c_if_z", "remote_c_if_h", "remote_c_if_sx", "remote_c_if_rx","remote_c_if_ry","remote_c_if_rz","remote_c_if_cx","remote_c_if_cy","remote_c_if_cz", "remote_c_if_ecr"
};

const std::unordered_map<std::string, std::string> CORRESPONDENCE_REMOTE_GATE_MAP = {
    {"remote_c_if_x", "x"},
    {"remote_c_if_y", "y"},
    {"remote_c_if_z", "z"},
    {"remote_c_if_h", "h"},
    {"remote_c_if_sx", "sx"},
    {"remote_c_if_rx", "rx"},
    {"remote_c_if_ry", "ry"},
    {"remote_c_if_rz", "rz"},
    {"remote_c_if_cx", "cx"},
    {"remote_c_if_cy", "cy"},
    {"remote_c_if_cz", "cz"},
    {"remote_c_if_ecr", "ecr"},
};

} // End namespace constants
} //End namespace cunqa





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