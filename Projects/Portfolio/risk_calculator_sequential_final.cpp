#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <fstream>
#include <iomanip>

// Function to determine the risk category based on the HEART score
std::string getRiskCategory(int heartScore) {
    if (heartScore <= 3) return "Low";
    if (heartScore <= 6) return "Moderate";
    return "High";
}

// Function to generate random high-risk patient data for a given number of patients
void generateHighRiskPatientData(int totalPatients, std::vector<std::string>& patientData, int& highRiskCount) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> scoreDist(0, 2);   // Each component of HEART score ranges from 0 to 2
    std::uniform_int_distribution<> idDist(10000, 99999); // Random 5-digit patient ID
    std::uniform_int_distribution<> bpDist(90, 180);   // Blood pressure range (systolic and diastolic)
    std::uniform_int_distribution<> hrDist(50, 120);   // Heart rate range
    std::uniform_real_distribution<> spo2Dist(90.0, 100.0); // SpO2 percentage
    std::uniform_real_distribution<> labValueDist(0.1, 10.0); // Random lab values

    for (int i = 0; i < totalPatients; ++i) {
        // Generate random HEART score components
        int historyScore = scoreDist(gen);
        int ecgScore = scoreDist(gen);
        int ageScore = scoreDist(gen);
        int riskFactorsScore = scoreDist(gen);
        int troponinScore = scoreDist(gen);

        // Generate additional random patient data
        int systolicBP = bpDist(gen);
        int diastolicBP = bpDist(gen);
        int heartRate = hrDist(gen);
        double spo2 = spo2Dist(gen);
        double randomLabValue = labValueDist(gen);

 // Calculate the HEART score
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
                   << std::setw(12) << std::fixed << std::setprecision(1) << spo2 << ","
                   << std::setw(12) << std::fixed << std::setprecision(1) << randomLabValue << ","
                   << std::setw(12) << category;
            patientData.push_back(record.str());
        }
    }
}

int main() {
    const int totalPatients = 1000000; // Total number of patients
    std::vector<std::string> highRiskPatients;
    int highRiskCount = 0;

    // Start timing
    auto start = std::chrono::high_resolution_clock::now();

    // Generate data
    generateHighRiskPatientData(totalPatients, highRiskPatients, highRiskCount);

    // Stop timing
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    double high_risk_percent = (double)highRiskCount / totalPatients * 100;
    // Output results
    std::cout << "Total high-risk patients: " << highRiskCount << " out of " << totalPatients << std::endl;
    std::cout << "Percentage of high-risk patients: " << high_risk_percent << "%" << std::endl;
    std::cout << "Time taken: " << elapsed.count() << " seconds" << std::endl;


    // Output the high-risk patients to a file
    std::ofstream outFile("high_risk_patients_sequential_EMR.csv");
    if (outFile.is_open()) {
        // Write headers
        outFile << std::setw(12) << "Patient_ID" << ","
                << std::setw(9) << "History" << ","
                << std::setw(12) << "ECG" << ","
                << std::setw(12) << "Age" << ","
                << std::setw(12) << "Risk_Factors" << ","
                << std::setw(12) << "Troponin" << ","
                << std::setw(12) << "HEART_Score" << ","
                << std::setw(12) << "Sys_BP" << ","
                << std::setw(12) << "Dia_BP" << ","
                << std::setw(12) << "Heart_Rate" << ","
                << std::setw(12) << "SpO2" << ","
                << std::setw(12) << "Lab_Value" << ","
                << std::setw(12) << "Risk_Level" << "\n";

        // Write patient data
        for (const auto& record : highRiskPatients) {
            outFile << record << "\n";
        }
        outFile.close();
        std::cout << "High-risk patient data written to high_risk_patients_sequential_EMR.csv" << std::endl;
    } else {
        std::cerr << "Error: Unable to open file for writing." << std::endl;
    }

    return 0;
}

