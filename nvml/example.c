#include <stdio.h>
#include "nvml.h"

const char * convertToComputeModeString(nvmlComputeMode_t mode)
{
    switch (mode)
    {
        case NVML_COMPUTEMODE_DEFAULT:
            return "Default";
        case NVML_COMPUTEMODE_EXCLUSIVE_THREAD:
            return "Exclusive_Thread";
        case NVML_COMPUTEMODE_PROHIBITED:
            return "Prohibited";
        case NVML_COMPUTEMODE_EXCLUSIVE_PROCESS:
            return "Exclusive Process";
        default:
            return "Unknown";
    }
}

int main()
{
    nvmlReturn_t result;
    unsigned int device_count, i;

    // First initialize NVML library
    result = nvmlInit();
    if (NVML_SUCCESS != result)
    { 
        printf("Failed to initialize NVML: %s\n", nvmlErrorString(result));

        printf("Press ENTER to continue...\n");
        getchar();
        return 1;
    }

    result = nvmlDeviceGetCount(&device_count);
    if (NVML_SUCCESS != result)
    { 
        printf("Failed to query device count: %s\n", nvmlErrorString(result));
        goto Error;
    }
    printf("Found %d device%s\n\n", device_count, device_count != 1 ? "s" : "");

    printf("Listing devices:\n");    
    for (i = 0; i < device_count; i++)
    {
        nvmlDevice_t device;
        char name[NVML_DEVICE_NAME_BUFFER_SIZE];
        nvmlPciInfo_t pci;
        nvmlComputeMode_t compute_mode;

        // Query for device handle to perform operations on a device
        // You can also query device handle by other features like:
        // nvmlDeviceGetHandleBySerial
        // nvmlDeviceGetHandleByPciBusId
        result = nvmlDeviceGetHandleByIndex(i, &device);
        if (NVML_SUCCESS != result)
        { 
            printf("Failed to get handle for device %i: %s\n", i, nvmlErrorString(result));
            goto Error;
        }

        result = nvmlDeviceGetName(device, name, NVML_DEVICE_NAME_BUFFER_SIZE);
        if (NVML_SUCCESS != result)
        { 
            printf("Failed to get name of device %i: %s\n", i, nvmlErrorString(result));
            goto Error;
        }
        
        // pci.busId is very useful to know which device physically you're talking to
        // Using PCI identifier you can also match nvmlDevice handle to CUDA device.
        result = nvmlDeviceGetPciInfo(device, &pci);
        if (NVML_SUCCESS != result)
        { 
            printf("Failed to get pci info for device %i: %s\n", i, nvmlErrorString(result));
            goto Error;
        }

        printf("%d. %s [%s]\n", i, name, pci.busId);

        // This is a simple example on how you can modify GPU's state
        result = nvmlDeviceGetComputeMode(device, &compute_mode);
        if (NVML_ERROR_NOT_SUPPORTED == result)
            printf("\t This is not CUDA capable device\n");
        else if (NVML_SUCCESS != result)
        { 
            printf("Failed to get compute mode for device %i: %s\n", i, nvmlErrorString(result));
            goto Error;
        }
        else
        {
            // try to change compute mode
            printf("\t Changing device's compute mode from '%s' to '%s'\n", 
                    convertToComputeModeString(compute_mode), 
                    convertToComputeModeString(NVML_COMPUTEMODE_PROHIBITED));

            result = nvmlDeviceSetComputeMode(device, NVML_COMPUTEMODE_PROHIBITED);
            if (NVML_ERROR_NO_PERMISSION == result)
                printf("\t\t Need root privileges to do that: %s\n", nvmlErrorString(result));
            else if (NVML_ERROR_NOT_SUPPORTED == result)
                printf("\t\t Compute mode prohibited not supported. You might be running on\n"
                       "\t\t windows in WDDM driver model or on non-CUDA capable GPU.\n");
            else if (NVML_SUCCESS != result)
            {
                printf("\t\t Failed to set compute mode for device %i: %s\n", i, nvmlErrorString(result));
                goto Error;
            } 
            else
            {
                printf("\t Restoring device's compute mode back to '%s'\n", 
                        convertToComputeModeString(compute_mode));
                result = nvmlDeviceSetComputeMode(device, compute_mode);
                if (NVML_SUCCESS != result)
                { 
                    printf("\t\t Failed to restore compute mode for device %i: %s\n", i, nvmlErrorString(result));
                    goto Error;
                }
            }
        }
    }

    result = nvmlShutdown();
    if (NVML_SUCCESS != result)
        printf("Failed to shutdown NVML: %s\n", nvmlErrorString(result));

    printf("All done.\n");

    printf("Press ENTER to continue...\n");
    getchar();
    return 0;

Error:
    result = nvmlShutdown();
    if (NVML_SUCCESS != result)
        printf("Failed to shutdown NVML: %s\n", nvmlErrorString(result));

    printf("Press ENTER to continue...\n");
    getchar();
    return 1;
}
