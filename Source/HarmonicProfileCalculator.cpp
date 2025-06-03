#include "HarmonicProfileCalculator.h"
#include <cmath>
#include <numeric>

namespace rosy {

//==============================================================================
// LookupTables Implementation
//==============================================================================

const HarmonicProfileCalculator::LookupTables& HarmonicProfileCalculator::LookupTables::getInstance()
{
    static LookupTables instance;
    return instance;
}

HarmonicProfileCalculator::LookupTables::LookupTables()
{
    // Pre-calculate binomial coefficients up to a reasonable size
    constexpr int maxN = 32;  // Adjust based on needs
    binomialCoeffs.reserve((maxN * (maxN + 1)) / 2);
    for (int n = 0; n < maxN; ++n)
    {
        for (int k = 0; k <= n/2; ++k)
        {
            binomialCoeffs.push_back(calculateBinomial(n, k));
        }
    }
}

int64_t HarmonicProfileCalculator::LookupTables::calculateBinomial(int n, int k)
{
    if (k > n) return 0;
    if (k == 0 || k == n) return 1;
    if (k > n - k) k = n - k;  // Take advantage of symmetry
    
    int64_t result = 1;
    for (int i = 0; i < k; ++i)
    {
        result *= (n - i);
        result /= (i + 1);
    }
    return result;
}

int64_t HarmonicProfileCalculator::LookupTables::getBinomial(int n, int k) const
{
    if (k > n) return 0;
    if (k == 0 || k == n) return 1;
    if (k > n - k) k = n - k;
    
    size_t index = (n * (n + 1) / 2) + k;
    return (index < binomialCoeffs.size()) ? binomialCoeffs[index] : calculateBinomial(n, k);
}

//==============================================================================
// HarmonicProfileCalculator Implementation
//==============================================================================

float HarmonicProfileCalculator::chebyshevCoefficient(int n, int i)
{
    // Early exit if i > n or parity doesn't match
    if (i > n || ((n - i) % 2 != 0)) return 0.0f;
    
    float coeff = 0.0f;
    int maxJ = (n - i) / 2;
    const auto& lookupTables = LookupTables::getInstance();
    
    for (int j = 0; j <= maxJ; ++j)
    {
        float term = std::powf(-1.0f, static_cast<float>(j)) * 
                    lookupTables.getBinomial(n - j - 1, j) * 
                    std::powf(2.0f, static_cast<float>(n - 2 * j - i));
        coeff += term;
    }
    
    return static_cast<float>(n) * coeff;
}

float HarmonicProfileCalculator::calculateCoefficient(int i, const std::vector<float>& harmonicGains)
{
    float coeff = 0.0f;
    for (size_t n = 1; n <= harmonicGains.size(); ++n)
    {
        coeff += harmonicGains[n-1] * chebyshevCoefficient(static_cast<int>(n), i);
    }
    return coeff;
}

std::vector<float> HarmonicProfileCalculator::calculateAllCoefficients(const std::vector<float>& harmonicGains)
{
    // Size needs to be highest harmonic + 1 to include all powers
    std::vector<float> coeffs(harmonicGains.size() + 1, 0.0f);
    
    for (size_t i = 0; i <= harmonicGains.size(); ++i)
    {
        coeffs[i] = calculateCoefficient(static_cast<int>(i), harmonicGains);
    }
    
    // Calculate the peak value by evaluating at x = 1
    // (which simplifies to just summing the coefficients)
    float peakValue = std::accumulate(coeffs.begin(), coeffs.end(), 0.0f);
    
    // Normalize coefficients to make peak value = 1
    if (std::abs(peakValue) > 1e-10f)  // Avoid division by zero
    {
        float normFactor = 1.0f / peakValue;
        for (float& coeff : coeffs)
        {
            coeff *= normFactor;
        }
    }
    
    return coeffs;
}

} // namespace rosy 