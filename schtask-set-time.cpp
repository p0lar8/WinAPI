/********************************************************************
 This schedules a task to start and stop on the times/dates you provide.

Must set:
Line 60 - Task name
Line 65 & 66 - exe path
Line 272 - End time of task
Line 278 - Start time of task

********************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#define _WIN32_DCOM

#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <comdef.h>
#include <wincred.h>
//  Include the task header file.
#include <taskschd.h>
#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsupp.lib")
#pragma comment(lib, "credui.lib")

using namespace std;


int __cdecl wmain()
{
    //  ------------------------------------------------------
    //  Initialize COM.
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        printf("\nCoInitializeEx failed: %x", hr);
        return 1;
    }

    //  Set general COM security levels.
    hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        0,
        NULL);

    if (FAILED(hr))
    {
        printf("\nCoInitializeSecurity failed: %x", hr);
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Create a name for the task.
    LPCWSTR wszTaskName = L"Time Trigger Test Task";

    //  Get the directory and set the path to your exe.

    
    wstring wstrExecutablePath = _wgetenv(L"WINDIR");
    wstrExecutablePath += L"\\SYSTEM32\\NOTEPAD.EXE";

    /*
    EXAMPLE:
    wstring wstrExecutablePath = _wgetenv(L"LOCALAPPDATA");
    wstrExecutablePath += L"\\blah.exe";
    */


    //  ------------------------------------------------------
    //  Create an instance of the Task Service. 
    ITaskService* pService = NULL;
    hr = CoCreateInstance(CLSID_TaskScheduler,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITaskService,
        (void**)&pService);
    if (FAILED(hr))
    {
        printf("Failed to create an instance of ITaskService: %x", hr);
        CoUninitialize();
        return 1;
    }

    //  Connect to the task service.
    hr = pService->Connect(_variant_t(), _variant_t(),
        _variant_t(), _variant_t());
    if (FAILED(hr))
    {
        printf("ITaskService::Connect failed: %x", hr);
        pService->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Get the pointer to the root task folder.  This folder will hold the
    //  new task that is registered.
    ITaskFolder* pRootFolder = NULL;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr))
    {
        printf("Cannot get Root folder pointer: %x", hr);
        pService->Release();
        CoUninitialize();
        return 1;
    }

    //  If the same task exists, remove it.
    pRootFolder->DeleteTask(_bstr_t(wszTaskName), 0);

    //  Create the task definition object to create the task.
    ITaskDefinition* pTask = NULL;
    hr = pService->NewTask(0, &pTask);

    pService->Release();  // COM clean up.  Pointer is no longer used.
    if (FAILED(hr))
    {
        printf("Failed to CoCreate an instance of the TaskService class: %x", hr);
        pRootFolder->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Get the registration info for setting the identification.
    IRegistrationInfo* pRegInfo = NULL;
    hr = pTask->get_RegistrationInfo(&pRegInfo);
    if (FAILED(hr))
    {
        printf("\nCannot get identification pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pRegInfo->put_Author(_bstr_t(L"Author Name"));
    pRegInfo->Release();
    if (FAILED(hr))
    {
        printf("\nCannot put identification info: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Create the principal for the task - these credentials
    //  are overwritten with the credentials passed to RegisterTaskDefinition
    IPrincipal* pPrincipal = NULL;
    hr = pTask->get_Principal(&pPrincipal);
    if (FAILED(hr))
    {
        printf("\nCannot get principal pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Set up principal logon type to interactive logon
    hr = pPrincipal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);
    pPrincipal->Release();
    if (FAILED(hr))
    {
        printf("\nCannot put principal info: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Create the settings for the task
    ITaskSettings* pSettings = NULL;
    hr = pTask->get_Settings(&pSettings);
    if (FAILED(hr))
    {
        printf("\nCannot get settings pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Set setting values for the task.  
    hr = pSettings->put_StartWhenAvailable(VARIANT_TRUE);
    pSettings->Release();
    if (FAILED(hr))
    {
        printf("\nCannot put setting information: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    // Set the idle settings for the task.
    IIdleSettings* pIdleSettings = NULL;
    hr = pSettings->get_IdleSettings(&pIdleSettings);
    if (FAILED(hr))
    {
        printf("\nCannot get idle setting information: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pIdleSettings->put_WaitTimeout(_bstr_t(L"PT5M"));
    pIdleSettings->Release();
    if (FAILED(hr))
    {
        printf("\nCannot put idle setting information: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }


    //  ------------------------------------------------------
    //  Get the trigger collection to insert the time trigger.
    ITriggerCollection* pTriggerCollection = NULL;
    hr = pTask->get_Triggers(&pTriggerCollection);
    if (FAILED(hr))
    {
        printf("\nCannot get trigger collection: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Add the time trigger to the task.
    ITrigger* pTrigger = NULL;
    hr = pTriggerCollection->Create(TASK_TRIGGER_DAILY, &pTrigger);
    pTriggerCollection->Release();
    if (FAILED(hr))
    {
        printf("\nCannot create trigger: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    IDailyTrigger* pDailyTrigger = NULL;
    hr = pTrigger->QueryInterface(
        IID_IDailyTrigger, (void**)&pDailyTrigger);
    pTrigger->Release();
    if (FAILED(hr))
    {
        printf("\nQueryInterface call failed for IDailyTrigger: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    hr = pDailyTrigger->put_Id(_bstr_t(L"Trigger1"));
    if (FAILED(hr))
        printf("\nCannot put trigger ID: %x", hr);

    hr = pDailyTrigger->put_EndBoundary(_bstr_t(L"2022-05-02T08:00:00"));
    if (FAILED(hr))
        printf("\nCannot put end boundary on trigger: %x", hr);

    //  Set the task to start at a certain time. The time 
    //  format should be YYYY-MM-DDTHH:MM:SS(+-)(timezone).
    hr = pDailyTrigger->put_StartBoundary(_bstr_t(L"2022-03-03T16:23:00"));
    pDailyTrigger->Release();
    if (FAILED(hr))
    {
        printf("\nCannot add start boundary to trigger: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }


    //  ------------------------------------------------------
    //  Add an action to the task. This task will execute notepad.exe.     
    IActionCollection* pActionCollection = NULL;

    //  Get the task action collection pointer.
    hr = pTask->get_Actions(&pActionCollection);
    if (FAILED(hr))
    {
        printf("\nCannot get Task collection pointer: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Create the action, specifying that it is an executable action.
    IAction* pAction = NULL;
    hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
    pActionCollection->Release();
    if (FAILED(hr))
    {
        printf("\nCannot create the action: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    IExecAction* pExecAction = NULL;
    //  QI for the executable task pointer.
    hr = pAction->QueryInterface(
        IID_IExecAction, (void**)&pExecAction);
    pAction->Release();
    if (FAILED(hr))
    {
        printf("\nQueryInterface call failed for IExecAction: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  Set the path of the executable to notepad.exe.
    hr = pExecAction->put_Path(_bstr_t(wstrExecutablePath.c_str()));
    pExecAction->Release();
    if (FAILED(hr))
    {
        printf("\nCannot put action path: %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    //  ------------------------------------------------------
    //  Save the task in the root folder.
    IRegisteredTask* pRegisteredTask = NULL;
    hr = pRootFolder->RegisterTaskDefinition(
        _bstr_t(wszTaskName),
        pTask,
        TASK_CREATE_OR_UPDATE,
        _variant_t(),
        _variant_t(),
        TASK_LOGON_INTERACTIVE_TOKEN,
        _variant_t(L""),
        &pRegisteredTask);
    if (FAILED(hr))
    {
        printf("\nError saving the Task : %x", hr);
        pRootFolder->Release();
        pTask->Release();
        CoUninitialize();
        return 1;
    }

    printf("\n Success! Task successfully registered. ");

    //  Clean up.
    pRootFolder->Release();
    pTask->Release();
    pRegisteredTask->Release();
    CoUninitialize();
    return 0;
}
