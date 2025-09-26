/**
 * Grab Fare Calculator
 *
 * This program calculates the fare for a Grab ride based on the selected
 * vehicle type, trip distance, trip duration, peak-hour surcharge and
 * promotional codes.  It demonstrates clean coding practices by splitting
 * the logic into reusable functions and using descriptive variable names.
 *
 * Vehicle types have different base rates and per-kilometre charges.  The
 * program also supports an optional booking fee, a minimum fare and a
 * multiplier for peak hours.  Promo codes can be applied to reduce the
 * subtotal up to a capped amount.  After computing the fare the program
 * prints a detailed breakdown for transparency.
 *
 * Date: 24 September 2025
 */

#include <iostream>
#include <iomanip>
#include <limits>
#include <map>
#include <string>
#include <cmath>

using std::cin;
using std::cout;
using std::endl;
using std::string;

// Structure to hold pricing for a vehicle type
struct Rates {
    double base;         // Base fare (RM)
    double perKm;        // Cost per kilometre (RM)
    double perMin;       // Optional cost per minute (RM) – set to 0 if unused
    double bookingFee;   // Fixed booking fee (RM)
};

// Structure to hold promo code information
struct Promo {
    double percentage;   // Percentage discount (0–1)
    double cap;          // Maximum discount amount (RM)
};

// Structure to hold a full fare breakdown
struct FareBreakdown {
    double base;
    double booking;
    double distanceCostOffPeak;
    double timeCost;
    double peakMultiplier;      // 1.0 or a value > 1 during peak times
    double distanceCostFinal;
    double subtotal;
    string promoCode;
    double discountApplied;
    double totalBeforeMin;
    double totalPayable;
};

// Convert a string to uppercase and trim whitespace
static string toUpperTrim(const string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    string trimmed = (start == string::npos) ? "" : s.substr(start, end - start + 1);
    for (auto &c : trimmed) {
        c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
    }
    return trimmed;
}

// Read a double with validation; returns false if input fails (EOF)
bool readPositiveDouble(const string &prompt, double &out, double maxVal = std::numeric_limits<double>::max()) {
    while (true) {
        cout << prompt;
        if (!(cin >> out)) {
            return false;
        }
        if (out > 0 && out <= maxVal) {
            return true;
        }
        cout << "Invalid input. Please enter a positive number";
        if (maxVal < std::numeric_limits<double>::max()) {
            cout << " (<= " << maxVal << ")";
        }
        cout << "." << endl;
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

// Read an integer menu choice between lo and hi inclusive
int readMenuChoice(const string &prompt, int lo, int hi) {
    int choice;
    while (true) {
        cout << prompt;
        if (cin >> choice && choice >= lo && choice <= hi) {
            return choice;
        }
        cout << "Invalid choice. Please enter a number between " << lo << " and " << hi << "." << endl;
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

// Compute the fare breakdown based on input parameters
FareBreakdown computeFare(double distanceKm, double timeMin, bool isPeak,
                          const string &promoCodeRaw, const Rates &rates,
                          double peakMultiplier, double minFare,
                          const std::map<string, Promo> &promoMap) {
    FareBreakdown fb{};
    fb.base = rates.base;
    fb.booking = rates.bookingFee;
    fb.distanceCostOffPeak = distanceKm * rates.perKm;
    fb.timeCost = timeMin * rates.perMin;
    fb.peakMultiplier = isPeak ? peakMultiplier : 1.0;
    fb.distanceCostFinal = fb.distanceCostOffPeak * fb.peakMultiplier;
    fb.subtotal = fb.base + fb.booking + fb.distanceCostFinal + fb.timeCost;

    // Determine promo code discount
    string code = toUpperTrim(promoCodeRaw);
    fb.promoCode = promoMap.count(code) ? code : "NONE";
    Promo promo = promoMap.at(fb.promoCode);
    double rawDiscount = fb.subtotal * promo.percentage;
    fb.discountApplied = (rawDiscount > promo.cap) ? promo.cap : rawDiscount;

    fb.totalBeforeMin = fb.subtotal - fb.discountApplied;
    fb.totalPayable = (fb.totalBeforeMin < minFare) ? minFare : fb.totalBeforeMin;

    // Round values to two decimal places
    auto round2 = [](double v) { return std::round(v * 100.0) / 100.0; };
    fb.base = round2(fb.base);
    fb.booking = round2(fb.booking);
    fb.distanceCostOffPeak = round2(fb.distanceCostOffPeak);
    fb.timeCost = round2(fb.timeCost);
    fb.distanceCostFinal = round2(fb.distanceCostFinal);
    fb.subtotal = round2(fb.subtotal);
    fb.discountApplied = round2(fb.discountApplied);
    fb.totalBeforeMin = round2(fb.totalBeforeMin);
    fb.totalPayable = round2(fb.totalPayable);
    return fb;
}

// Print the fare breakdown in a user-friendly format
void printBreakdown(const FareBreakdown &fb) {
    cout << std::fixed << std::setprecision(2);
    cout << "\n--- Fare Breakdown (RM) ---" << endl;
    cout << "Base fare              : " << fb.base << endl;
    cout << "Booking fee            : " << fb.booking << endl;
    cout << "Distance cost (off-peak): " << fb.distanceCostOffPeak << endl;
    if (fb.peakMultiplier > 1.0) {
        cout << "Peak multiplier x" << fb.peakMultiplier << " applied to distance" << endl;
    } else {
        cout << "Peak multiplier        : x1.00 (off-peak)" << endl;
    }
    cout << "Distance cost (final)  : " << fb.distanceCostFinal << endl;
    if (fb.timeCost > 0) {
        cout << "Time cost              : " << fb.timeCost << endl;
    }
    cout << "Subtotal               : " << fb.subtotal << endl;
    cout << "Promo code used        : " << fb.promoCode;
    if (fb.promoCode != "NONE") {
        cout << " (discount " << fb.discountApplied << ")";
    }
    cout << endl;
    cout << "Total before min fare  : " << fb.totalBeforeMin << endl;
    cout << "Minimum fare enforced  : " << fb.totalPayable << endl;
    cout << "-----------------------------" << endl;
    cout << "Total payable          : " << fb.totalPayable << "\n" << endl;
}

int main() {
    // Define vehicle types and their rates.  These values roughly reflect
    // real-world Grab fares in Malaysia (update them as needed).
    std::map<int, Rates> vehicles{
        {1, {2.50, 1.20, 0.20, 1.00}},  // GrabCar Economy
        {2, {4.00, 1.60, 0.30, 1.00}},  // GrabCar Premium
        {3, {1.50, 0.50, 0.00, 0.50}}   // GrabBike
    };

    // Define promo codes and their discount caps
    std::map<string, Promo> promoMap{
        {"NONE",      {0.00, 0.00}},
        {"GRAB10",    {0.10, 3.00}}, // 10% off up to RM3
        {"STUDENT15", {0.15, 5.00}}, // 15% off up to RM5
        {"SUPER20",   {0.20, 8.00}}  // 20% off up to RM8
    };

    const double peakMultiplier = 1.50; // 50% surcharge on distance cost
    const double minFare = 5.00;        // Minimum payable fare

    cout << "Grab Fare Calculator (Enhanced)" << endl;
    cout << "Promo codes available: NONE, GRAB10, STUDENT15, SUPER20" << endl;

    bool runAgain = true;
    while (runAgain) {
        // Display menu
        cout << "\nSelect vehicle type:" << endl;
        cout << "1) GrabCar Economy" << endl;
        cout << "2) GrabCar Premium" << endl;
        cout << "3) GrabBike" << endl;

        int vehicleChoice = readMenuChoice("Enter choice (1–3): ", 1, 3);
        Rates selectedRates = vehicles.at(vehicleChoice);
        string vehicleName;
        switch (vehicleChoice) {
            case 1: vehicleName = "GrabCar Economy"; break;
            case 2: vehicleName = "GrabCar Premium"; break;
            case 3: vehicleName = "GrabBike"; break;
            default: vehicleName = "Unknown"; break;
        }

        cout << "Selected: " << vehicleName << endl;
        cout << "Base fare: RM " << selectedRates.base
             << ", Per km: RM " << selectedRates.perKm
             << ", Booking fee: RM " << selectedRates.bookingFee;
        if (selectedRates.perMin > 0) {
            cout << ", Per minute: RM " << selectedRates.perMin;
        }
        cout << endl;

        // Read trip details
        double distanceKm;
        if (!readPositiveDouble("Enter trip distance (km): ", distanceKm, 200.0)) {
            cout << "Input ended unexpectedly. Exiting." << endl;
            return 0;
        }
        double timeMin = 0;
        if (selectedRates.perMin > 0) {
            if (!readPositiveDouble("Enter estimated time (minutes): ", timeMin, 1000.0)) {
                cout << "Input ended unexpectedly. Exiting." << endl;
                return 0;
            }
        }

        // Determine peak or off-peak
        int peakChoice = readMenuChoice("Is this a peak‑hour ride? 1) No  2) Yes : ", 1, 2);
        bool isPeak = (peakChoice == 2);

        // Ask for promo code
        cout << "Enter promo code (or NONE): ";
        string promoInput;
        cin >> promoInput;

        // Compute fare
        FareBreakdown fb = computeFare(distanceKm, timeMin, isPeak,
                                      promoInput, selectedRates,
                                      peakMultiplier, minFare,
                                      promoMap);

        // Print summary
        cout << "\n=== Summary =================================\n";
        cout << "Vehicle: " << vehicleName << " | "
             << (isPeak ? "Peak" : "Off-peak") << " | Distance: "
             << std::fixed << std::setprecision(2) << distanceKm << " km";
        if (selectedRates.perMin > 0) cout << " | Time: " << timeMin << " min";
        cout << "\n=============================================\n";

        printBreakdown(fb);

        // Ask if user wants another calculation
        int again = readMenuChoice("Would you like to calculate another fare? 1) Yes  2) No : ", 1, 2);
        runAgain = (again == 1);
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    cout << "Thank you for using Grab Fare Calculator. Have a nice day!" << endl;
    return 0;
}
