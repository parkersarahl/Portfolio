#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <fstream>
#include <mpi.h>
#include <iomanip>

// Function to calculate the HEART score risk category based on the score
std::string getRiskCategory(int heartScore) {
    if (heartScore <= 3) {
        return "Low";
    } else if (heartScore <= 6) {
        return "Moderate";
    } else {
        return "High";
    }
}

// Function to generate random high-risk patient data for a given range of patients
void generateHighRiskPatientData(int start, int end, std::vector<std::string>& patientData, int &highRiskCount) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> scoreDist(0, 2);   // Each component of HEART score ranges from 0 to 2
    std::uniform_int_distribution<> idDist(10000, 99999); // Random 5-digit patient ID
    std::uniform_int_distribution<> bpDist(90, 180);
    std::uniform_int_distribution<> hrDist(50, 120);
    std::uniform_real_distribution<> spo2Dist(90.0, 100.0);
    std::uniform_real_distribution<> labValueDist(0.1, 10.0);

    for (int i = start; i < end; ++i) {
        //Generate random HEART score componenets
        int historyScore = scoreDist(gen);
        int ecgScore = scoreDist(gen);
        int ageScore = scoreDist(gen);
        int riskFactorsScore = scoreDist(gen);
        int troponinScore = scoreDist(gen);


        //Generate additional random patient data
        int systolicBP = bpDist(gen);
        int diastolicBP = bpDist(gen);
        int heartRate = hrDist(gen);
        double spo2 = spo2Dist(gen);
        double randomLabValue = labValueDist(gen);

        int heartScore = historyScore + ecgScore + ageScore + riskFactorsScore + troponinScore;
        std::string category = getRiskCategory(heartScore);

        if (category == "High") {
            highRiskCount++;  // Increment the high-risk counter
            int patientID = idDist(gen);
            std::ostringstream record;
            record << std::setw(12) << patientID << ","
                   << std::setw(9) << historyScore << ","
                   << std::setw(12) << ecgScore << ","
                   << std::setw(12) << ageScore << ","
                   << std::setw(12) << riskFactorsScore << ","
                   << std::setw(12) << troponinScore << ","
                   << std::setw(12) << heartScore << ","
                   << std::setw(12) << systolicBP << ","
                   << std::setw(12) << diastolicBP << ","
                   << std::setw(12) << heartRate << ","
                   << std::setw(12) << std::fixed << std::setprecision(1)
                   << std::setw(12) << std::fixed << std::setprecision(1)
                   << std::setw(12) << category << ",";
            patientData.push_back(record.str());
        }
    }
}

int main(int argc, char* argv[]) {
    const int numPatients = 1000000;   // Total number of patients to generate
    int rank, size;

    // Initialize MPI environment
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Start timing
    double startTime = MPI_Wtime();

    // Calculate workload for each process
    const int patientsPerProcess = numPatients / size;
    int start = rank * patientsPerProcess;
    int end = (rank == size - 1) ? numPatients : start + patientsPerProcess;

    std::vector<std::string> localPatientData;
    int localHighRiskCount = 0;
    generateHighRiskPatientData(start, end, localPatientData, localHighRiskCount);

    // Gather high-risk counts from all processes
    int globalHighRiskCount = 0;
    MPI_Reduce(&localHighRiskCount, &globalHighRiskCount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    int localDataSize = localPatientData.size();
    std::vector<int> recvCounts(size);
    MPI_Gather(&localDataSize, 1, MPI_INT, recvCounts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<std::string> globalPatientData;
    if (rank == 0) {
        int totalDataSize = 0;
        for (int i = 0; i < size; ++i) {
            totalDataSize += recvCounts[i];
        }
        globalPatientData.resize(totalDataSize);
    }

    std::string flatData;
    for (const auto& record : localPatientData) {
        flatData += record + "\n";
    }
   int localFlatDataSize = flatData.size();
    std::vector<int> flatRecvCounts(size);
    MPI_Gather(&localFlatDataSize, 1, MPI_INT, flatRecvCounts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<char> globalFlatData;
    std::vector<int> displs(size);
    if (rank == 0) {
        int offset = 0;
        for (int i = 0; i < size; ++i) {
            displs[i] = offset;
            offset += flatRecvCounts[i];
        }
        globalFlatData.resize(offset);
    }

    MPI_Gatherv(flatData.data(), localFlatDataSize, MPI_CHAR,
                globalFlatData.data(), flatRecvCounts.data(), displs.data(), MPI_CHAR, 0, MPI_COMM_WORLD);

    // Root process writes the gathered data to a CSV file
    if (rank == 0) {
        std::ofstream outFile("high_risk_patient_data_MPI_EMR.csv");
        if (outFile.is_open()) {
        outFile << std::setw(12) <<"Patient_ID" << ","
                << std::setw(9) << "History" << ","
                << std::setw(5) << "ECG" << ","
                << std::setw(5) << "Age" <<  ","
                << std::setw(14) << "Risk_Factors" << ","
                << std::setw(10) << "Troponin" << ","
                << std::setw(13) << "HEART_Score" << ","
                << std::setw(12) << "Sys_BP" << ","
                << std::setw(12) << "Dia_BP" << ","
                << std::setw(12) << "Heart_Rate" << ","
                << std::setw(12) << "SPO2" << ","
                << std::setw(12) << "Lab_Value" << ","
                << std::setw(9) << "Risk_Level" << "\n";

        outFile.write(globalFlatData.data(), globalFlatData.size());
        }
        outFile.close();
        std::cout << "High-risk patient data saved to high_risk_patient_data_mpi_EMR.csv" << std::endl;

        double high_risk_percent = (double)globalHighRiskCount / numPatients * 100;
 // Output the total number of high-risk patients and the execution time
        std::cout << "Total high-risk patients: " << globalHighRiskCount << " out of " << numPatients << std::endl;
        std::cout << "Percent of high-risk patients: " << high_risk_percent << "%" << std::endl;
    }

    // End timing
    double endTime = MPI_Wtime();
    if (rank == 0) {
        std::cout << "Total execution time: " << endTime - startTime << " seconds" << std::endl;
    }

    MPI_Finalize();
    return 0;
}

