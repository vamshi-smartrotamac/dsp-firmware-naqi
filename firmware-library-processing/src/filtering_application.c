#include "filtering_application.h"
#include "filtering_utils.h"
#include "filters_coefficients_emg.h"
#include "filters_coefficients_eog.h"

// clang-format off
// EMG filter coefficients
float bpIIR_sections[EMG_NUMBER_SECTIONS_BP][FLTR_UTILS_NUMBER_COEFFICIENTS_PER_SECTION] = 
{
    {EMG_SOS_BP_60_5_100_SECTION_1},
    {EMG_SOS_BP_60_5_100_SECTION_2},
    {EMG_SOS_BP_60_5_100_SECTION_3},
    {EMG_SOS_BP_60_5_100_SECTION_4},
    {EMG_SOS_BP_60_5_100_SECTION_5},
    {EMG_SOS_BP_60_5_100_SECTION_6},
    {EMG_SOS_BP_60_5_100_SECTION_7},
    {EMG_SOS_BP_60_5_100_SECTION_8},
    {EMG_SOS_BP_60_5_100_SECTION_9}
};
float bpIIR_init_cdt[EMG_NUMBER_SECTIONS_BP][FLTR_UTILS_NUMBER_COEFFICIENTS_PER_SECTION_INIT_CDT] = 
{
    {EMG_INIT_CDT_BP_60_5_100_SECTION_1},
    {EMG_INIT_CDT_BP_60_5_100_SECTION_2},
    {EMG_INIT_CDT_BP_60_5_100_SECTION_3},
    {EMG_INIT_CDT_BP_60_5_100_SECTION_4},
    {EMG_INIT_CDT_BP_60_5_100_SECTION_5},
    {EMG_INIT_CDT_BP_60_5_100_SECTION_6},
    {EMG_INIT_CDT_BP_60_5_100_SECTION_7},
    {EMG_INIT_CDT_BP_60_5_100_SECTION_8},
    {EMG_INIT_CDT_BP_60_5_100_SECTION_9}
};

// EOG filter coefficients
float hpIIR_sections[EOG_NUMBER_SECTIONS_HP][FLTR_UTILS_NUMBER_COEFFICIENTS_PER_SECTION] = 
{
    {EOG_SOS_HP_0_14_SECTION_1},
    {EOG_SOS_HP_0_14_SECTION_2},
    {EOG_SOS_HP_0_14_SECTION_3}
};
float hpIIR_init_cdt[EOG_NUMBER_SECTIONS_HP][FLTR_UTILS_NUMBER_COEFFICIENTS_PER_SECTION_INIT_CDT] = 
{
    {EOG_INIT_CDT_HP_0_14_SECTION_1},
    {EOG_INIT_CDT_HP_0_14_SECTION_2},
    {EOG_INIT_CDT_HP_0_14_SECTION_3}
};
float lpIIR_sections[EOG_NUMBER_SECTIONS_LP][FLTR_UTILS_NUMBER_COEFFICIENTS_PER_SECTION] = 
{
    {EOG_SOS_LP_8_SECTION_1},
    {EOG_SOS_LP_8_SECTION_2},
    {EOG_SOS_LP_8_SECTION_3}
};
float lpIIR_init_cdt[EOG_NUMBER_SECTIONS_LP][FLTR_UTILS_NUMBER_COEFFICIENTS_PER_SECTION_INIT_CDT] = 
{
    {EOG_INIT_CDT_LP_8_SECTION_1},
    {EOG_INIT_CDT_LP_8_SECTION_2},
    {EOG_INIT_CDT_LP_8_SECTION_3}
};

/* EMG BandPass Filter */
static FilterIIR filter_emg = { 
    EMG_NUMBER_SECTIONS_BP,      // numberSections
    EMG_PADLEN_BP_60_5_100_IIR,   // padLen
    (float *)bpIIR_sections,     // sosSections
    (float *)bpIIR_init_cdt      // initialConditions
};

static FilterIIR filter_eog[] = {
    { /* EOG High-Pass Filter */
        EOG_NUMBER_SECTIONS_HP,      // numberSections
        EOG_PADLEN_HP_0_14_IIR,      // padLen
        (float *)hpIIR_sections,     // sosSections
        (float *)hpIIR_init_cdt      // initialConditions
    },
    { /* EOG Low-Pass Filter */
        EOG_NUMBER_SECTIONS_LP,      // numberSections
        EOG_PADLEN_LP_8_IIR,         // padLen
        (float *)lpIIR_sections,     // sosSections
        (float *)lpIIR_init_cdt      // initialConditions
    },
};
// clang-format on

int8_t fltr_app_runFilteringEMG(float *inputData, size_t sizeInputData)
{
    return fltr_utils_iirSosFiltFilt(inputData, sizeInputData, &filter_emg);
}

int8_t fltr_app_runFilteringEOG(float *inputData, size_t sizeInputData)
{
    for (uint8_t i = 0; i < 2; ++i)
    {
        if (fltr_utils_iirSosFiltFilt(inputData, sizeInputData, &filter_eog[i]) < 0)
        {
            return -1;
        };
    }
    return 0;
}