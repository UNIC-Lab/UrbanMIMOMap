#include <stdio.h>
#include <ctime>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <direct.h> // For _mkdir on Windows
#include <stdlib.h> // For atoi

#include "outdoor_propagation_runms_mimo.h"

#ifndef API_DATA_FOLDER
#define API_DATA_FOLDER "../../api/winprop/data/"
#endif // !API_DATA_FOLDER

#ifndef PROJ_FOLDER
#define PROJ_FOLDER "./"
#endif // !PROJ_FOLDER

#ifdef _WIN32
#include <windows.h>
#endif

// Define constants
//mapIndRange:0-499
//antIndRange:0-39

constexpr float TX_AZIMUTH =                60.f;           // 发射天线方位角（度）

constexpr int START_MAP_INDEX =             298;            // 地图起始索引（包含）
constexpr int END_MAP_INDEX =               298;            // 地图终止索引（包含）
constexpr int START_ANTENNA_INDEX =         1;              // 天线起始索引（包含）
constexpr int END_ANTENNA_INDEX =           1;              // 天线终止索引（包含）

constexpr float PREDICTION_HEIGHT =         1.5;            // 接收天线离地高度（米）
constexpr float RESOLUTION_M =              0.5f;           // 结果矩阵分辨率（米）
constexpr float TX_TILT =                   80.f;           // 发射天线俯仰角（度，0度为竖直向下，90度为水平向右）
constexpr float RX_AZIMUTH =                0.f;            // 接收天线方位角（度，0度为东向，90度为北向）
constexpr float RX_TILT =                   180.f;          // 接收天线俯仰角（度，0度为竖直向上，90度为水平向右）
constexpr int NUM_TX_ANTENNAS_PER_ARRAY =   4;              // 每阵列发射天线数量
constexpr int NUM_RX_ANTENNAS_PER_ARRAY =   4;              // 每阵列接收天线数量
constexpr float ANTENNA_SPACING =           0.5f;           // 天线间距（波长的倍数）
constexpr float ANTENNA_HEIGHT =            15.f;           // 发射天线离地高度（米）
constexpr float ANTENNA_POWER =             23.f;           // 天线发射功率（dBm）
constexpr float ANTENNA_FREQ =              3500.f;         // 工作频率（MHz）
constexpr float CHANNEL_BANDWIDTH =         50000000.f;     // 信道带宽（Hz）
constexpr bool PYTHON_COMPRESS =            true;           // 启用Python结果矩阵压缩
constexpr bool COUPLING_FLAG =              false;          // 启用耦合效应


// Function declarations (Callback functions are already declared in the provided code)
int _STD_CALL CallbackMessage(const char* Text);
int _STD_CALL CallbackError(const char* Text, int Error);
int _STD_CALL CallbackProgress(int value, const char* text);
void write_ascii(const WinProp_Result* Resultmatrix, const char* Filename);

int main(int argc, char** argv)
{
    //开始计时
    clock_t start, end;
    start = clock();
    // 打印常量信息
    std::cout << "TX_AZIMUTH: " << TX_AZIMUTH << std::endl;
    std::cout << "START_MAP_INDEX: " << START_MAP_INDEX << std::endl;
    std::cout << "END_MAP_INDEX: " << END_MAP_INDEX << std::endl;
    std::cout << "START_ANTENNA_INDEX: " << START_ANTENNA_INDEX << std::endl;
    std::cout << "END_ANTENNA_INDEX: " << END_ANTENNA_INDEX << std::endl;
    std::cout << "PREDICTION_HEIGHT: " << PREDICTION_HEIGHT << std::endl;
    std::cout << "RESOLUTION_M: " << RESOLUTION_M << std::endl;
    std::cout << "TX_TILT: " << TX_TILT << std::endl;
    std::cout << "RX_AZIMUTH: " << RX_AZIMUTH << std::endl;
    std::cout << "RX_TILT: " << RX_TILT << std::endl;
    std::cout << "NUM_TX_ANTENNAS_PER_ARRAY: " << NUM_TX_ANTENNAS_PER_ARRAY << std::endl;
    std::cout << "NUM_RX_ANTENNAS_PER_ARRAY: " << NUM_RX_ANTENNAS_PER_ARRAY << std::endl;
    std::cout << "ANTENNA_SPACING: " << ANTENNA_SPACING << std::endl;
    std::cout << "ANTENNA_HEIGHT: " << ANTENNA_HEIGHT << std::endl;
    std::cout << "ANTENNA_POWER: " << ANTENNA_POWER << std::endl;
    std::cout << "ANTENNA_FREQ: " << ANTENNA_FREQ << std::endl;
    std::cout << "CHANNEL_BANDWIDTH: " << CHANNEL_BANDWIDTH << std::endl;
    std::cout << "PYTHON_COMPRESS: " << PYTHON_COMPRESS << std::endl;
    std::cout << "\n\n" << std::endl;


    // --- Initialisation ---

    int Error = 0;
    WinProp_ParaMain GeneralParameters;
    WinProp_Antenna Antenna;
    WinProp_Callback Callback;
    WinProp_Result Resultmatrix;
    WinProp_RayMatrix RayMatrix;
    WinProp_Propagation_Results OutputResults;

    // --- Configuration ---
    // Use defined constants
    int start_map_index = START_MAP_INDEX;
    int end_map_index = END_MAP_INDEX;
    int start_antenna_index = START_ANTENNA_INDEX;
    int end_antenna_index = END_ANTENNA_INDEX;

    // Base paths for odb files, coordinate files, and result directories
    const std::string odb_base_path = PROJ_FOLDER "odb_files/";                             // Base path for odb files
    const std::string xy_texts_base_path = PROJ_FOLDER "/antenna_txt/";                     // Base path for coordinate files
    const std::string result_base_path = PROJ_FOLDER "/resu/";                              // Base path for result directories
    const std::string result_runMS = PROJ_FOLDER "/resu/runMS";                             // Base path for runMS result directories
    //const std::string pattern_file = API_DATA_FOLDER "antennas/WiMax_3500MHz_60deg.apb";    // Antenna pattern file
    //const std::string pattern_file = API_DATA_FOLDER "antennas/dipole.apa";    // Antenna pattern file
    const std::string pattern_file = API_DATA_FOLDER "antennas/Beams2Control4Data.ffe";    // Antenna pattern file
    //const std::string pattern_file = API_DATA_FOLDER "antennas/Antenna.apb";    // Antenna pattern file

    // --- Map Loop ---
    for (int map_index = start_map_index; map_index <= end_map_index; ++map_index)
    {
        // Construct odb file path
        std::stringstream odb_file_ss;
        //odb_file_ss << odb_base_path << map_index << "_odb";
        odb_file_ss << odb_base_path << map_index;
        std::string odb_file_path = odb_file_ss.str();

        // Construct coordinate file path
        std::stringstream xy_file_ss;
        //xy_file_ss << xy_texts_base_path << map_index << "_unenclosed.txt";
        xy_file_ss << xy_texts_base_path << map_index << ".txt";
        std::string xy_file_path = xy_file_ss.str();

        // Construct result directory path for current map
        std::stringstream resu_map_dir_ss;
        //resu_map_dir_ss << result_base_path << "resu_map" << map_index;
        resu_map_dir_ss << result_base_path;
        std::string resu_map_dir_path = resu_map_dir_ss.str();

        // Construct result_runMS directory path for current map
        std::stringstream resu_runMS_dir_ss;
        resu_runMS_dir_ss << result_runMS;
        std::string resu_runMS_dir_path = resu_runMS_dir_ss.str();

        // create result_base_path if it doesn't exist
        if (_mkdir(result_base_path.c_str()) == -1) {
            if (errno != EEXIST) { // EEXIST means directory already exists, which is okay
                std::cerr << "Error creating directory " << result_base_path << std::endl;
                return 1; // Exit if directory creation fails (and it's not because it already exists)
            }
        }
        // Create result directory if it doesn't exist
        if (_mkdir(resu_map_dir_path.c_str()) == -1) {
            if (errno != EEXIST) { // EEXIST means directory already exists, which is okay
                std::cerr << "Error creating directory " << resu_map_dir_path << std::endl;
                return 1; // Exit if directory creation fails (and it's not because it already exists)
            }
        }
        // Create result_runMS directory if it doesn't exist
        if (_mkdir(resu_runMS_dir_path.c_str()) == -1) {
            if (errno != EEXIST) { // EEXIST means directory already exists, which is okay
                std::cerr << "Error creating directory " << resu_runMS_dir_path << std::endl;
                return 1; // Exit if directory creation fails (and it's not because it already exists)
            }
        }

        // --- Read antenna coordinates from file ---
        std::ifstream xy_file(xy_file_path.c_str());
        if (!xy_file.is_open()) {
            std::cerr << "Error opening coordinate file: " << xy_file_path << std::endl;
            continue; // Skip to the next map if coordinate file is not found
        }

        // --- Antenna Loop ---
        for (int antenna_index = start_antenna_index; antenna_index <= end_antenna_index; ++antenna_index)
        {
            // --- Initialisation of parameters (reset for each antenna) ---
            WinProp_Structure_Init_ParameterMain(&GeneralParameters);
            WinProp_Structure_Init_Antenna(&Antenna);
            WinProp_Structure_Init_Callback(&Callback);
            WinProp_Structure_Init_Result(&Resultmatrix);
            WinProp_Structure_Init_RayMatrix(&RayMatrix);
            WinProp_Structure_Init_Propagation_Results(&OutputResults);

            /*---------------- Definition of scenario -------------------------------------*/
            /* Definition of general parameters. */
            GeneralParameters.ScenarioMode = SCENARIOMODE_URBAN; // Urban prediction
            GeneralParameters.PredictionModelUrban = PREDMODEL_UDP; // Use Dominant Path Model

            /* Definition of prediction area. (These values are fixed, you might need to adjust based on your odb files) */
            GeneralParameters.UrbanLowerLeftX = 0;      // x轴起始坐标
            GeneralParameters.UrbanLowerLeftY = 0;      // y轴起始坐标
            GeneralParameters.UrbanUpperRightX = 256;   // x轴终止坐标
            GeneralParameters.UrbanUpperRightY = 256;   // y轴终止坐标
            double PredictionHeight = PREDICTION_HEIGHT; // Prediction height in meter
            /* Size of matrix with results. */
            double resolution = RESOLUTION_M; // Resolution in meter
            GeneralParameters.Resolution = resolution;                  // Resolution in meter
            GeneralParameters.NrLayers = 1;                             // Number of prediction heights
            GeneralParameters.PredictionHeights = &PredictionHeight;    // Prediction height in meter

            /* Building vector data and topography. */
            GeneralParameters.BuildingsMode = BUILDINGSMODE_BINARY; // load a .odb database (urban + topography)
            sprintf(GeneralParameters.BuildingsName, "%s", odb_file_path.c_str()); // Vector database in WinProp format (using current map's odb file)

            /*--------------------------- Definition of antenna ---------------------------*/
            /* Position and configuration. */
            double antennaHeight = ANTENNA_HEIGHT;  // Antenna height in meter
            double antennaPower = ANTENNA_POWER;    // Antenna power in dBm
            double antennaFrequency = ANTENNA_FREQ; // Antenna frequency in MHz

            Antenna.Id = antenna_index + 1;         // Antenna ID, make it unique within each map if needed
            Antenna.SiteId = map_index + 1;         // Site ID, representing the map index
            Antenna.Height = antennaHeight;         // Height of the antenna above the ground in meter
            Antenna.Power = antennaPower;           // Power in dBm
            Antenna.Frequency = antennaFrequency;   // Frequency in MHz

            // --- Read antenna coordinates from file ---
            std::string line;
            xy_file.clear();  // Clear any error flags
            xy_file.seekg(0); // Rewind to beginning of file

            bool found = false;
            for (int i = 0; i <= antenna_index; ++i) {
                if (!std::getline(xy_file, line)) {
                    std::cerr << "Error reading antenna coordinate from file for antenna index "
                        << antenna_index << " in map " << map_index << std::endl;
                    break;
                }
                // Skip empty lines
                if (line.empty()) continue;

                // Only process the line we're interested in
                if (i == antenna_index) {
                    found = true;
                    // Trim whitespace from line
                    line.erase(line.find_last_not_of(" \n\r\t") + 1);

                    // Parse coordinates
                    size_t open = line.find('(');
                    size_t comma = line.find(',');
                    size_t close = line.find(')');

                    if (open != std::string::npos &&
                        comma != std::string::npos &&
                        close != std::string::npos &&
                        open < comma && comma < close) {

                        try {
                            Antenna.Longitude_X = std::stod(line.substr(open + 1, comma - open - 1));
                            Antenna.Latitude_Y = std::stod(line.substr(comma + 1, close - comma - 1));

                            std::stringstream antenna_name_ss;
                            /*antenna_name_ss << map_index << "_X" << Antenna.Longitude_X
                                << "_Y" << Antenna.Latitude_Y << "_ami" << TX_AZIMUTH;*/
                            antenna_name_ss << map_index << "_" << antenna_index << "_" << TX_AZIMUTH; // Unique antenna name
                            strncpy(Antenna.Name, antenna_name_ss.str().c_str(), sizeof(Antenna.Name) - 1);
                            Antenna.Name[sizeof(Antenna.Name) - 1] = '\0';
                        }
                        catch (...) {
                            std::cerr << "Error parsing coordinates from line: " << line << std::endl;
                            found = false;
                        }
                    }
                    else {
                        std::cerr << "Invalid coordinate format in line: " << line << std::endl;
                        found = false;
                    }
                }
            }

            if (!found) {
                Antenna.Longitude_X = 0.0;
                Antenna.Latitude_Y = 0.0;
                std::cerr << "Using default coordinates for antenna " << antenna_index << std::endl;
            }

            /* Definition of outputs to be computed and written in WinProp format. */
            GeneralParameters.OutputResults = &OutputResults;

            std::stringstream output_path_ss;
            output_path_ss << resu_map_dir_path; // Output to the map specific directory
            sprintf(OutputResults.ResultPath, "%s", output_path_ss.str().c_str()); // Output data directory

            OutputResults.FieldStrength = 0;            // enable field strength output
            OutputResults.PathLoss = 0;                 // enable path loss output
            OutputResults.StatusLOS = 0;                // enable line-of-sight status output
            OutputResults.RayFilePropPaths = 0;         // enable ray file propagation paths output
            OutputResults.StrFilePropPaths = 0;         // enable string file propagation paths output
            OutputResults.AdditionalResultsASCII = 0;   // enable additional results in ASCII format

            /*-------------------------- Callbacks ----------------------------------------*/
            Callback.Percentage = CallbackProgress;
            Callback.Message = CallbackMessage;
            Callback.Error = CallbackError;

            // 输出所有的参数信息
            //Antenna.print();  // 天线



            /*----------------------- Compute outdoor prediction --------------------------*/
            std::cout << "Computing outdoor prediction for antenna " << antenna_index << " in map " << map_index << std::endl;
            clock_t t_start_per_antenna, t_end_per_antenna;
            t_start_per_antenna = clock();
            Error = OutdoorPlugIn_ComputePrediction(
                &Antenna, &GeneralParameters, NULL, 0, NULL, NULL, NULL, NULL, &Callback, &Resultmatrix, &RayMatrix, NULL, NULL);

            /*----------------------- Do something with results ---------------------------*/
            if (Error == 0) {
                //std::stringstream result_filename_ss;
                //result_filename_ss << resu_map_dir_path << "/PowerResults_antenna" << antenna_index << ".txt"; // Unique filename for each antenna
                //std::string result_filename = result_filename_ss.str();
                //write_ascii(&Resultmatrix, result_filename.c_str()); // Write results to file in ASCII format
            }
            else {
                /* Error during prediction. Print error message. */
                CallbackError(GeneralParameters.ErrorMessageMain, Error);
            }



            /* ===============================================================================*/
            /*----------------------- Rx antenna superposition (RunMS) --------------------*/
            /* ===============================================================================*/
            // Create project
            WinProp_SuperposeMS superposeMS;

            /* Definition of antenna pattern. */
            WinProp_Pattern     WinPropPattern;
            WinProp_Structure_Init_Pattern(&WinPropPattern);
            WinPropPattern.Mode = PATTERN_MODE_FILE; // Load pattern from file
            //sprintf(WinPropPattern.Filename, "%s", API_DATA_FOLDER "antennas/Antenna.apb"); // Pattern file (including extension)
            sprintf(WinPropPattern.Filename, "%s", pattern_file.c_str()); // Pattern file (including extension)


            // -------------------------设置发射和接收天线阵列默认参数
            WINPROP_MS_POLARIZATION_TYPE pol1 = WINPROP_MS_POLARIZATION_VERTICAL;   // 垂直极化
            WINPROP_MS_POLARIZATION_TYPE pol2 = WINPROP_MS_POLARIZATION_HORIZONTAL; // 水平极化
            WINPROP_MS_POLARIZATION_TYPE pol3 = WINPROP_MS_POLARIZATION_X_POL;      // 双极化
            WINPROP_MS_ARRAY_TYPE txArrayAPI = WINPROP_MS_ARRAY_SINGLE_ANTENNA;     // 默认只考虑单天线发射天线阵列
            WINPROP_MS_ARRAY_TYPE rxArrayAPI = WINPROP_MS_ARRAY_SINGLE_ANTENNA;     // 默认只考虑单天线接收天线阵列

            // --------------------------设置发射和接收天线实际参数
            const int numTxnumPerTxArray = NUM_TX_ANTENNAS_PER_ARRAY;               // 发射天线数量
            const int numRxnumPerRxArray = NUM_RX_ANTENNAS_PER_ARRAY;               // 接收天线数量
            WINPROP_MS_POLARIZATION_TYPE txPol = pol3;      // 发射天线的极化方式
            WINPROP_MS_POLARIZATION_TYPE rxPol = pol3;      // 接收天线的极化方式
            if (numTxnumPerTxArray > 1) {
                txArrayAPI = WINPROP_MS_ARRAY_LINEAR;       // 发射天线阵列为线性阵列
            }
            if (numRxnumPerRxArray > 1) {
                rxArrayAPI = WINPROP_MS_ARRAY_LINEAR;       // 接收天线阵列为线性阵列
            }
            // 设置发射天线阵列
            const double txAzimuth = TX_AZIMUTH;            // 发射天线方位角
            const double txTilt = TX_TILT;                  // 发射天线倾角
            if (Error == 0)
                Error = superposeMS.setArray(false, txArrayAPI,
                    numTxnumPerTxArray,                     // 天线数量
                    txAzimuth,                              // 整个天线阵列的方位角
                    ANTENNA_SPACING,                        // 天线之间的间距，设置为0.5倍波长
                    0.f,                                    // 圆形天线阵列的半径
                    COUPLING_FLAG                           // 是否考虑天线耦合
                );
            // 设置发射天线阵列中每个单元的参数
            if (Error == 0) {
                for (int i = 0; i < numTxnumPerTxArray; i++) {
                    Error = superposeMS.setArrayElement(
                        false,                              // True=接收单元，False=发射单元
                        i,                                  // 单元索引（0~3）
                        //nullptr,                            // 单元的方向图文件
                        &WinPropPattern,                    // 单元的方向图文件
                        nullptr,                            // 全极化参数
                        txAzimuth,                          // 单元方位角（0°=东，90°=北）
                        txTilt,                             // 单元下倾角（0°=竖直向下，90°=水平）
                        txPol,                              // 极化方式
                        { 0., 0., 0. }                      // 单元偏移（仅与单元类型为WINPROP_MS_ARRAY_INDIVIDUAL相关）
                    );
                }
            }

            // 设置接收天线阵列
            const double rxAzimuth = RX_AZIMUTH;            // 接收天线方位角
            const double rxTilt = RX_TILT;                  // 接收天线倾角
            if (Error == 0)
                Error = superposeMS.setArray(true, rxArrayAPI,
                    numRxnumPerRxArray          /*rxArray.nbrElements*/,
                    rxAzimuth                   /*rxArray.arrayAzimuth*/,
                    ANTENNA_SPACING             /*rxArray.spacingHorizontal*/,
                    0.f                         /*rxArray.radius*/,
                    COUPLING_FLAG               /*rxArray.antCoupling != 0*/
                );
            // 设置接收天线阵列中每个单元的参数
            if (Error == 0) {
                // azimuth with east over north orientation, in case of trajectories 0deg means 90deg right
                // from driving direction, using 180deg here to point the Rx antenna towards the Tx 90deg tilt
                // is equal to horizontal orientation.
                for (int i = 0; i < numRxnumPerRxArray; i++) {
                    Error = superposeMS.setArrayElement(
                        true,                   /*True=接收单元*/
                        i,                      /*单元索引（0~3）*/
                        //nullptr,
                        &WinPropPattern,        /*单元的方向图文件*/
                        nullptr,                /*全极化参数*/
                        rxAzimuth               /*azimuth*/,
                        rxTilt                  /*tilt*/,
                        rxPol,                  /*polarization*/
                        { 0., 0., 0. }          /*offset*/
                    );
                }
            }

            // 设置计算区域
            if (Error == 0)
                Error = superposeMS.setPropagationArea(0, Antenna, RayMatrix);      // 设置计算区域为整个天线阵列

            // Set computation parameter
            float channelBandwidth = CHANNEL_BANDWIDTH;                             // 信道带宽
            WinProp_MS_Para msPara;
            WinProp_Structure_Init_MS_Para(&msPara);                                // 初始化计算参数
            msPara.coherentSuperposition = 1;                                       // 启用相干叠加
            msPara.channelType = WINPROP_MS_CHANNEL_PROPAGATION;                    // 从射线数据确定的信道数据幅度和相位
            msPara.channelBandwidth = channelBandwidth;                             // 信道带宽
            msPara.channelNormalization = WINPROP_MS_NORMALIZE_TIME_FROBENIUS;      // 信道归一化方式为时域Frobenius
            //msPara.channelNormalization = WINPROP_MS_NORMALIZE_NONE;                
            msPara.snirMode = WINPROP_MS_SNIR_CALC;                                 // 计算SNIR

            // Enable ouputs
            WinProp_MS_AdditionalResults msAdditionalResult;
            WinProp_Structure_Init_MS_AdditionalResults(&msAdditionalResult);                       // 初始化输出参数
            strcpy(msAdditionalResult.outputPath, result_runMS.c_str());                            // 设置输出文件夹路径
            msAdditionalResult.channelMatricesPerPoint = 1;                                         // 输出每个点的信道矩阵
            msAdditionalResult.channelMatricesPerRay = 0;                                           // 输出每个线的信道矩阵
            msAdditionalResult.channelMatrixResultMode = WINPROP_MS_CHANNEL_MATRIX_REAL_IMAG;       // 输出为实数和虚数的形式
            //msAdditionalResult.channelMatrixResultMode = WINPROP_MS_CHANNEL_MATRIX_MAG_PHASE;     // 输出为幅度和相位的形式
            msAdditionalResult.propagationPaths = 0;                                                // 输出paths结果
            msAdditionalResult.channelCapacity = 0;                                                 // 输出信道容量

            // 计算
            std::cout << "Starting RunMS of antenna " << Antenna.Name << std::endl;   // 开始计算
            if (Error == 0)
                Error = superposeMS.compute(msPara, &msAdditionalResult, &Callback);

            /*----------------------------- Free memory -----------------------------------*/
            WinProp_FreeResult(&Resultmatrix);
            WinProp_FreeRayMatrix(&RayMatrix);

            // 处理结果，保存为png图片
            if (Error == 0) {
                // 信道矩阵文件名
                std::stringstream result_filename_ss;
                result_filename_ss << result_runMS << "/" << Antenna.Name << " ChannelMatricesPerPoint.txt";
                std::string result_matrix_filename = result_filename_ss.str();
                // fpp文件名
                std::stringstream fpp_file_name_ss;
                fpp_file_name_ss << result_runMS << "/" << Antenna.Name << " Power.fpp";
                std::string fpp_file_name = fpp_file_name_ss.str();
                // 后处理之前的fpp文件
                std::stringstream fpp_file_name_before_ss;
                fpp_file_name_before_ss << resu_map_dir_path << "/" << Antenna.Name << " Power.fpp";
                std::string fpp_file_name_before = fpp_file_name_before_ss.str();

                // --- 调用 Python EXE 处理 TXT 文件 ---
                if (PYTHON_COMPRESS) {
                    std::stringstream python_command_ss;
                    // 假设你的 Python EXE 位于 C++ 可执行文件相对路径的 'dist' 文件夹中，
                    // 并且命名为 'process_data.exe'。根据需要调整路径。
                    //python_command_ss << "./dist/process_data.exe \"" << result_filename << "\""; // 如果文件名包含空格，则用引号括起来
                    //python_command_ss << PROJ_FOLDER << "matrix_to_png_2_2mimo_exe.exe \"" << result_matrix_filename << "\"";
                    python_command_ss << PROJ_FOLDER << "matrix2npz.exe \"" << result_matrix_filename << "\"";
                    std::string python_command = python_command_ss.str();

                    std::cout << "\n执行 Python 脚本: " << python_command << std::endl;
                    int python_result = std::system(python_command.c_str()); // 执行命令

                    if (python_result == 0) {
                        std::cout << "Python 脚本执行成功。" << std::endl;

                        // --- 删除 TXT 文件 ---
                        std::remove(result_matrix_filename.c_str());

                        // ---删除runMS中的fpp文件---
                        std::remove(fpp_file_name.c_str());

                        // --- 删除runMS之前的fpp文件 ---
                        std::remove(fpp_file_name_before.c_str());

                    }
                    else {
                        std::cerr << "Python 脚本执行失败，返回码: " << python_result << std::endl;
                    }
                }
            }
            // 打印单个天线执行时间
            t_end_per_antenna = clock();
            printf("Time used for antenna %d: %.2f seconds\n", antenna_index, (double)(t_end_per_antenna - t_start_per_antenna) / CLOCKS_PER_SEC);
            // 打印已用总时间
            printf("Time used for all antennas: %.2f seconds\n", (double)(t_end_per_antenna - start) / CLOCKS_PER_SEC);
            // 打印剩余时间
            int num_antenna_all = (end_map_index - start_map_index + 1) * (end_antenna_index - start_antenna_index + 1); // 总天线数
            int num_antenna_done = (map_index - start_map_index) * 40 + antenna_index - start_antenna_index + 1; // 已完成天线数
            int num_antenna_remain = num_antenna_all - num_antenna_done; // 剩余天线数
            double time_remain = (double)(t_end_per_antenna - t_start_per_antenna) / CLOCKS_PER_SEC * num_antenna_remain; // 剩余时间

            // 正确的时间分解逻辑
            double hour_remain = 0.0;
            double minute_remain = 0.0;
            double second_remain = time_remain;

            // 先处理小时
            if (time_remain >= 3600.0) {
                hour_remain = floor(time_remain / 3600.0);  // 取小时整数部分
                double remaining_after_hours = fmod(time_remain, 3600.0);

                // 再处理分钟
                if (remaining_after_hours >= 60.0) {
                    minute_remain = floor(remaining_after_hours / 60.0);
                    second_remain = fmod(remaining_after_hours, 60.0);
                }
                else {
                    second_remain = remaining_after_hours;
                }
            }
            // 处理分钟
            else if (time_remain >= 60.0) {
                minute_remain = floor(time_remain / 60.0);
                second_remain = fmod(time_remain, 60.0);
            }

            // 输出格式修正（保留2位小数）
            printf("Time remaining: %.2f hours, %.2f minutes, %.2f seconds\n\n\n",
                hour_remain, minute_remain, second_remain);

        } // --- End Antenna Loop ---
        xy_file.close(); // Close coordinate file after processing all antennas for the map

    } // --- End Map Loop ---

    //结束计时
    end = clock();
    printf("\nTime used: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    return 0;
}


// --- Callback function implementations (same as in the original code) ---
int _STD_CALL CallbackMessage(const char* Text)
{
    if (Text == nullptr)
        return 0;

    //std::cout << "\n" << Text;

    return(0);
}

int _STD_CALL CallbackError(const char* Text, int Error)
{
    if (Text == nullptr)
        return 0;

    std::cout << "\n";

#ifdef __LINUX
    std::cout << "\033[31m" << "Error (" << Error << "): "; // highlight error in red color
#else
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
    std::cout << "Error (" << Error << "): ";
#endif // __LINUX
    //std::cout << Text;

#ifdef __LINUX
    std::cout << "\033[0m"; // highlight error in red color
#else
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
#endif // __LINUX

    return 0;
}

int _STD_CALL CallbackProgress(int value, const char* text)
{
    char Line[200];

    sprintf(Line, "\n%d%% %s", value, text);
    //std::cout << Line;

    return(0);
}

void write_ascii(const WinProp_Result* Resultmatrix, const char* Filename) {
    FILE* OutputFile = fopen(Filename, "w");
    if (OutputFile)
    {
        /* Loop all pixels. */
        for (int x = 0; x < Resultmatrix->Columns; x++)
        {
            for (int y = 0; y < Resultmatrix->Lines; y++)
            {
                /* Compute real coordinates. */
                double Coordinate_X = Resultmatrix->LowerLeftX + ((double)x + 0.5) * Resultmatrix->Resolution;
                double Coordinate_Y = Resultmatrix->LowerLeftY + ((double)y + 0.5) * Resultmatrix->Resolution;

                /* Check if pixel was computed or not */
                if (Resultmatrix->Matrix[0][x][y] > -1000)
                    fprintf(OutputFile, "%.2f\t%.2f\t%.2f\n", Coordinate_X, Coordinate_Y, Resultmatrix->Matrix[0][x][y]);
            }
        }

        /* Close file. */
        fclose(OutputFile);
    }
    else
        printf("\nCould not open the File: %s for writing.\n", Filename);
}
