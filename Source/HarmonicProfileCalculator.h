#pragma once

#include <JuceHeader.h>
#include <vector>
#include <cstdint>

namespace rosy {

/**
 * @brief Static utility class for calculating polynomial waveshaping coefficients from harmonic gains.
 * 
 * This class is implemented as a pure static utility class - it cannot be instantiated and all methods
 * are static. This design choice was made because the class maintains no state between calls and
 * performs pure mathematical transformations.
 * 
 * The class uses an internal singleton LookupTables class to cache commonly used binomial coefficients.
 * The singleton ensures that:
 * 1. The lookup table is only created once, on first use
 * 2. All calls to HarmonicProfileCalculator share the same lookup table
 * 3. The table persists for the lifetime of the program
 * 
 * Thread Safety: The class is thread-safe. The internal LookupTables singleton handles its own
 * thread-safe initialization, and all other methods are pure mathematical operations with no shared state.
 */
class HarmonicProfileCalculator
{
public:
    /** 
     * @brief Calculates all polynomial coefficients for the given harmonic gains.
     * 
     * @param harmonicGains Vector of gains for each harmonic, where index 0 is the fundamental
     * @return Vector of polynomial coefficients, where index is the power of x
     */
    static std::vector<float> calculateAllCoefficients(const std::vector<float>& harmonicGains);
    
    /**
     * @brief Calculates a single polynomial coefficient given all harmonic gains.
     * 
     * @param i The power of x for which to calculate the coefficient
     * @param harmonicGains Vector of gains for each harmonic
     * @return The coefficient for x^i in the polynomial
     */
    static float calculateCoefficient(int i, const std::vector<float>& harmonicGains);

private:
    // Prevent instantiation of this utility class
    HarmonicProfileCalculator() = delete;
    
    /**
     * @brief Internal singleton class for caching binomial coefficients.
     * 
     * This class is implemented as a singleton to ensure:
     * 1. Lazy initialization - tables are only created when first needed
     * 2. Single instance - all calculations share the same lookup table
     * 3. Automatic cleanup - instance is destroyed when program ends
     * 
     * The singleton pattern is implemented using C++11's magic statics,
     * which guarantees thread-safe initialization.
     */
    class LookupTables {
    public:
        static const LookupTables& getInstance();
        int64_t getBinomial(int n, int k) const;

    private:
        LookupTables();
        static int64_t calculateBinomial(int n, int k);
        std::vector<int64_t> binomialCoeffs;
    };

    /**
     * @brief Calculates the contribution of the nth Chebyshev polynomial to the ith power of x.
     * 
     * Uses the cached binomial coefficients from LookupTables for efficiency.
     */
    static float chebyshevCoefficient(int n, int i);
};

} // namespace rosy 