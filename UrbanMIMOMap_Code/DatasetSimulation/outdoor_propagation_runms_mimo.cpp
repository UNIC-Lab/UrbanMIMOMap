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

constexpr float TX_AZIMUTH =                60.f;           // �������߷�λ�ǣ��ȣ�

constexpr int START_MAP_INDEX =             298;            // ��ͼ��ʼ������������
constexpr int END_MAP_INDEX =               298;            // ��ͼ��ֹ������������
constexpr int START_ANTENNA_INDEX =         1;              // ������ʼ������������
constexpr int END_ANTENNA_INDEX =           1;              // ������ֹ������������

constexpr float PREDICTION_HEIGHT =         1.5;            // ����������ظ߶ȣ��ף�
constexpr float RESOLUTION_M =              0.5f;           // �������ֱ��ʣ��ף�
constexpr float TX_TILT =                   80.f;           // �������߸����ǣ��ȣ�0��Ϊ��ֱ���£�90��Ϊˮƽ���ң�
constexpr float RX_AZIMUTH =                0.f;            // �������߷�λ�ǣ��ȣ�0��Ϊ����90��Ϊ����
constexpr float RX_TILT =                   180.f;          // �������߸����ǣ��ȣ�0��Ϊ��ֱ���ϣ�90��Ϊˮƽ���ң�
constexpr int NUM_TX_ANTENNAS_PER_ARRAY =   4;              // ÿ���з�����������
constexpr int NUM_RX_ANTENNAS_PER_ARRAY =   4;              // ÿ���н�����������
constexpr float ANTENNA_SPACING =           0.5f;           // ���߼�ࣨ�����ı�����
constexpr float ANTENNA_HEIGHT =            15.f;           // ����������ظ߶ȣ��ף�
constexpr float ANTENNA_POWER =             23.f;           // ���߷��书�ʣ�dBm��
constexpr float ANTENNA_FREQ =              3500.f;         // ����Ƶ�ʣ�MHz��
constexpr float CHANNEL_BANDWIDTH =         50000000.f;     // �ŵ�����Hz��
constexpr bool PYTHON_COMPRESS =            true;           // ����Python�������ѹ��
constexpr bool COUPLING_FLAG =              false;          // �������ЧӦ


// Function declarations (Callback functions are already declared in the provided code)
int _STD_CALL CallbackMessage(const char* Text);
int _STD_CALL CallbackError(const char* Text, int Error);
int _STD_CALL CallbackProgress(int value, const char* text);
void write_ascii(const WinProp_Result* Resultmatrix, const char* Filename);

int main(int argc, char** argv)
{
    //��ʼ��ʱ
    clock_t start, end;
    start = clock();
    // ��ӡ������Ϣ
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
            GeneralParameters.UrbanLowerLeftX = 0;      // x����ʼ����
            GeneralParameters.UrbanLowerLeftY = 0;      // y����ʼ����
            GeneralParameters.UrbanUpperRightX = 256;   // x����ֹ����
            GeneralParameters.UrbanUpperRightY = 256;   // y����ֹ����
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

            // ������еĲ�����Ϣ
            //Antenna.print();  // ����



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


            // -------------------------���÷���ͽ�����������Ĭ�ϲ���
            WINPROP_MS_POLARIZATION_TYPE pol1 = WINPROP_MS_POLARIZATION_VERTICAL;   // ��ֱ����
            WINPROP_MS_POLARIZATION_TYPE pol2 = WINPROP_MS_POLARIZATION_HORIZONTAL; // ˮƽ����
            WINPROP_MS_POLARIZATION_TYPE pol3 = WINPROP_MS_POLARIZATION_X_POL;      // ˫����
            WINPROP_MS_ARRAY_TYPE txArrayAPI = WINPROP_MS_ARRAY_SINGLE_ANTENNA;     // Ĭ��ֻ���ǵ����߷�����������
            WINPROP_MS_ARRAY_TYPE rxArrayAPI = WINPROP_MS_ARRAY_SINGLE_ANTENNA;     // Ĭ��ֻ���ǵ����߽�����������

            // --------------------------���÷���ͽ�������ʵ�ʲ���
            const int numTxnumPerTxArray = NUM_TX_ANTENNAS_PER_ARRAY;               // ������������
            const int numRxnumPerRxArray = NUM_RX_ANTENNAS_PER_ARRAY;               // ������������
            WINPROP_MS_POLARIZATION_TYPE txPol = pol3;      // �������ߵļ�����ʽ
            WINPROP_MS_POLARIZATION_TYPE rxPol = pol3;      // �������ߵļ�����ʽ
            if (numTxnumPerTxArray > 1) {
                txArrayAPI = WINPROP_MS_ARRAY_LINEAR;       // ������������Ϊ��������
            }
            if (numRxnumPerRxArray > 1) {
                rxArrayAPI = WINPROP_MS_ARRAY_LINEAR;       // ������������Ϊ��������
            }
            // ���÷�����������
            const double txAzimuth = TX_AZIMUTH;            // �������߷�λ��
            const double txTilt = TX_TILT;                  // �����������
            if (Error == 0)
                Error = superposeMS.setArray(false, txArrayAPI,
                    numTxnumPerTxArray,                     // ��������
                    txAzimuth,                              // �����������еķ�λ��
                    ANTENNA_SPACING,                        // ����֮��ļ�࣬����Ϊ0.5������
                    0.f,                                    // Բ���������еİ뾶
                    COUPLING_FLAG                           // �Ƿ����������
                );
            // ���÷�������������ÿ����Ԫ�Ĳ���
            if (Error == 0) {
                for (int i = 0; i < numTxnumPerTxArray; i++) {
                    Error = superposeMS.setArrayElement(
                        false,                              // True=���յ�Ԫ��False=���䵥Ԫ
                        i,                                  // ��Ԫ������0~3��
                        //nullptr,                            // ��Ԫ�ķ���ͼ�ļ�
                        &WinPropPattern,                    // ��Ԫ�ķ���ͼ�ļ�
                        nullptr,                            // ȫ��������
                        txAzimuth,                          // ��Ԫ��λ�ǣ�0��=����90��=����
                        txTilt,                             // ��Ԫ����ǣ�0��=��ֱ���£�90��=ˮƽ��
                        txPol,                              // ������ʽ
                        { 0., 0., 0. }                      // ��Ԫƫ�ƣ����뵥Ԫ����ΪWINPROP_MS_ARRAY_INDIVIDUAL��أ�
                    );
                }
            }

            // ���ý�����������
            const double rxAzimuth = RX_AZIMUTH;            // �������߷�λ��
            const double rxTilt = RX_TILT;                  // �����������
            if (Error == 0)
                Error = superposeMS.setArray(true, rxArrayAPI,
                    numRxnumPerRxArray          /*rxArray.nbrElements*/,
                    rxAzimuth                   /*rxArray.arrayAzimuth*/,
                    ANTENNA_SPACING             /*rxArray.spacingHorizontal*/,
                    0.f                         /*rxArray.radius*/,
                    COUPLING_FLAG               /*rxArray.antCoupling != 0*/
                );
            // ���ý�������������ÿ����Ԫ�Ĳ���
            if (Error == 0) {
                // azimuth with east over north orientation, in case of trajectories 0deg means 90deg right
                // from driving direction, using 180deg here to point the Rx antenna towards the Tx 90deg tilt
                // is equal to horizontal orientation.
                for (int i = 0; i < numRxnumPerRxArray; i++) {
                    Error = superposeMS.setArrayElement(
                        true,                   /*True=���յ�Ԫ*/
                        i,                      /*��Ԫ������0~3��*/
                        //nullptr,
                        &WinPropPattern,        /*��Ԫ�ķ���ͼ�ļ�*/
                        nullptr,                /*ȫ��������*/
                        rxAzimuth               /*azimuth*/,
                        rxTilt                  /*tilt*/,
                        rxPol,                  /*polarization*/
                        { 0., 0., 0. }          /*offset*/
                    );
                }
            }

            // ���ü�������
            if (Error == 0)
                Error = superposeMS.setPropagationArea(0, Antenna, RayMatrix);      // ���ü�������Ϊ������������

            // Set computation parameter
            float channelBandwidth = CHANNEL_BANDWIDTH;                             // �ŵ�����
            WinProp_MS_Para msPara;
            WinProp_Structure_Init_MS_Para(&msPara);                                // ��ʼ���������
            msPara.coherentSuperposition = 1;                                       // ������ɵ���
            msPara.channelType = WINPROP_MS_CHANNEL_PROPAGATION;                    // ����������ȷ�����ŵ����ݷ��Ⱥ���λ
            msPara.channelBandwidth = channelBandwidth;                             // �ŵ�����
            msPara.channelNormalization = WINPROP_MS_NORMALIZE_TIME_FROBENIUS;      // �ŵ���һ����ʽΪʱ��Frobenius
            //msPara.channelNormalization = WINPROP_MS_NORMALIZE_NONE;                
            msPara.snirMode = WINPROP_MS_SNIR_CALC;                                 // ����SNIR

            // Enable ouputs
            WinProp_MS_AdditionalResults msAdditionalResult;
            WinProp_Structure_Init_MS_AdditionalResults(&msAdditionalResult);                       // ��ʼ���������
            strcpy(msAdditionalResult.outputPath, result_runMS.c_str());                            // ��������ļ���·��
            msAdditionalResult.channelMatricesPerPoint = 1;                                         // ���ÿ������ŵ�����
            msAdditionalResult.channelMatricesPerRay = 0;                                           // ���ÿ���ߵ��ŵ�����
            msAdditionalResult.channelMatrixResultMode = WINPROP_MS_CHANNEL_MATRIX_REAL_IMAG;       // ���Ϊʵ������������ʽ
            //msAdditionalResult.channelMatrixResultMode = WINPROP_MS_CHANNEL_MATRIX_MAG_PHASE;     // ���Ϊ���Ⱥ���λ����ʽ
            msAdditionalResult.propagationPaths = 0;                                                // ���paths���
            msAdditionalResult.channelCapacity = 0;                                                 // ����ŵ�����

            // ����
            std::cout << "Starting RunMS of antenna " << Antenna.Name << std::endl;   // ��ʼ����
            if (Error == 0)
                Error = superposeMS.compute(msPara, &msAdditionalResult, &Callback);

            /*----------------------------- Free memory -----------------------------------*/
            WinProp_FreeResult(&Resultmatrix);
            WinProp_FreeRayMatrix(&RayMatrix);

            // ������������ΪpngͼƬ
            if (Error == 0) {
                // �ŵ������ļ���
                std::stringstream result_filename_ss;
                result_filename_ss << result_runMS << "/" << Antenna.Name << " ChannelMatricesPerPoint.txt";
                std::string result_matrix_filename = result_filename_ss.str();
                // fpp�ļ���
                std::stringstream fpp_file_name_ss;
                fpp_file_name_ss << result_runMS << "/" << Antenna.Name << " Power.fpp";
                std::string fpp_file_name = fpp_file_name_ss.str();
                // ����֮ǰ��fpp�ļ�
                std::stringstream fpp_file_name_before_ss;
                fpp_file_name_before_ss << resu_map_dir_path << "/" << Antenna.Name << " Power.fpp";
                std::string fpp_file_name_before = fpp_file_name_before_ss.str();

                // --- ���� Python EXE ���� TXT �ļ� ---
                if (PYTHON_COMPRESS) {
                    std::stringstream python_command_ss;
                    // ������� Python EXE λ�� C++ ��ִ���ļ����·���� 'dist' �ļ����У�
                    // ��������Ϊ 'process_data.exe'��������Ҫ����·����
                    //python_command_ss << "./dist/process_data.exe \"" << result_filename << "\""; // ����ļ��������ո���������������
                    //python_command_ss << PROJ_FOLDER << "matrix_to_png_2_2mimo_exe.exe \"" << result_matrix_filename << "\"";
                    python_command_ss << PROJ_FOLDER << "matrix2npz.exe \"" << result_matrix_filename << "\"";
                    std::string python_command = python_command_ss.str();

                    std::cout << "\nִ�� Python �ű�: " << python_command << std::endl;
                    int python_result = std::system(python_command.c_str()); // ִ������

                    if (python_result == 0) {
                        std::cout << "Python �ű�ִ�гɹ���" << std::endl;

                        // --- ɾ�� TXT �ļ� ---
                        std::remove(result_matrix_filename.c_str());

                        // ---ɾ��runMS�е�fpp�ļ�---
                        std::remove(fpp_file_name.c_str());

                        // --- ɾ��runMS֮ǰ��fpp�ļ� ---
                        std::remove(fpp_file_name_before.c_str());

                    }
                    else {
                        std::cerr << "Python �ű�ִ��ʧ�ܣ�������: " << python_result << std::endl;
                    }
                }
            }
            // ��ӡ��������ִ��ʱ��
            t_end_per_antenna = clock();
            printf("Time used for antenna %d: %.2f seconds\n", antenna_index, (double)(t_end_per_antenna - t_start_per_antenna) / CLOCKS_PER_SEC);
            // ��ӡ������ʱ��
            printf("Time used for all antennas: %.2f seconds\n", (double)(t_end_per_antenna - start) / CLOCKS_PER_SEC);
            // ��ӡʣ��ʱ��
            int num_antenna_all = (end_map_index - start_map_index + 1) * (end_antenna_index - start_antenna_index + 1); // ��������
            int num_antenna_done = (map_index - start_map_index) * 40 + antenna_index - start_antenna_index + 1; // �����������
            int num_antenna_remain = num_antenna_all - num_antenna_done; // ʣ��������
            double time_remain = (double)(t_end_per_antenna - t_start_per_antenna) / CLOCKS_PER_SEC * num_antenna_remain; // ʣ��ʱ��

            // ��ȷ��ʱ��ֽ��߼�
            double hour_remain = 0.0;
            double minute_remain = 0.0;
            double second_remain = time_remain;

            // �ȴ���Сʱ
            if (time_remain >= 3600.0) {
                hour_remain = floor(time_remain / 3600.0);  // ȡСʱ��������
                double remaining_after_hours = fmod(time_remain, 3600.0);

                // �ٴ������
                if (remaining_after_hours >= 60.0) {
                    minute_remain = floor(remaining_after_hours / 60.0);
                    second_remain = fmod(remaining_after_hours, 60.0);
                }
                else {
                    second_remain = remaining_after_hours;
                }
            }
            // �������
            else if (time_remain >= 60.0) {
                minute_remain = floor(time_remain / 60.0);
                second_remain = fmod(time_remain, 60.0);
            }

            // �����ʽ����������2λС����
            printf("Time remaining: %.2f hours, %.2f minutes, %.2f seconds\n\n\n",
                hour_remain, minute_remain, second_remain);

        } // --- End Antenna Loop ---
        xy_file.close(); // Close coordinate file after processing all antennas for the map

    } // --- End Map Loop ---

    //������ʱ
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
