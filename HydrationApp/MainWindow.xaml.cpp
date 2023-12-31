// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <bcrypt.h>
#include <cfapi.h>
#include <comdef.h>

using namespace winrt;

#define FILE_SHARE_VALID_FLAGS 0x00000007

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::HydrationApp::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();
    }

    Windows::Foundation::IAsyncAction MainWindow::StartButton_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        //StartButton().IsEnabled(false);

        co_await HydrateFileAsync(FilePath().Text());

        //StartButton().IsEnabled(true);
        //FilePath().Text(L"C:\\Users\\yizzho\\OneDrive - Microsoft\\TEST\\more_kermits.png");

        co_return;
    }

    void MainWindow::CancelButton_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        CancelButton().IsEnabled(false);

        CancelHydration(FilePath().Text());

        CancelButton().IsEnabled(true);
    }

    Windows::Foundation::IAsyncAction MainWindow::HydrateFileAsync(std::wstring_view filePath)
    {
        PrintHydrationOutput(L"=== Try hydrate ===");

        auto keepThisAlive{ get_strong() };

        /* Resume on a background thread here since HydrateFile()
         * will block the calling thread (ie. it will cause UI to hang). */
        co_await winrt::resume_background();

        bool isRequestSuccessful = HydrateFile(filePath);

        co_await m_uiThread;

        PrintHydrationOutput(isRequestSuccessful ? L"=== Successfully sent request ===\n" : L"=== Failed to send request ===\n");

        co_return;
    }

    bool MainWindow::HydrateFile(std::wstring_view filePath)
    {
        MyLog(__FUNCTION__": === Starting ===\n");

        HydrationRequestVariables hydrationRequestVariables;

        //winrt::handle placeholder(CreateFile(filePath.data(), 0, FILE_READ_DATA, nullptr, OPEN_EXISTING, 0, nullptr));
        //m_placeholderHandle.reset(CreateFile(filePath.data(), 0, FILE_SHARE_VALID_FLAGS, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr));
        hydrationRequestVariables.PlaceholderHandle = std::make_shared<winrt::handle>(CreateFile(filePath.data(), 0, FILE_READ_DATA, nullptr, OPEN_EXISTING, 0, nullptr));

        //if (m_placeholderHandle.get() == INVALID_HANDLE_VALUE)
        if (hydrationRequestVariables.PlaceholderHandle.get()->get() == INVALID_HANDLE_VALUE)
        {
            _com_error err = GetLastError();
            LPCTSTR errMsg = err.ErrorMessage();

            MyLog(__FUNCTION__": ", errMsg, "\n");
        }
        else
        {
            MyLog(__FUNCTION__": Sending request for placeholder\n");
            LARGE_INTEGER offset;
            offset.QuadPart = 0;
            LARGE_INTEGER lengthOfEntireFile;
            lengthOfEntireFile.QuadPart = -1;

            hydrationRequestVariables.isRequestSuccessful = true;
            m_map[filePath] = hydrationRequestVariables;

            //auto result = CfHydratePlaceholder(m_placeholderHandle.get(), offset, lengthOfEntireFile, CF_HYDRATE_FLAG_NONE, &m_overlappedHydration);
            auto result = CfHydratePlaceholder(hydrationRequestVariables.PlaceholderHandle.get()->get(), offset, lengthOfEntireFile, CF_HYDRATE_FLAG_NONE, NULL);

            hydrationRequestVariables.PlaceholderHandle.get()->close();
            m_map.erase(filePath);
            
            // Note: this SUCCEED check isn't very accurate, so we still resume code after this, it's just a reminder
            if (!SUCCEEDED(result))
            {
                MyLog(__FUNCTION__": !!!SUCCEEDED(result) returns false\n");
                _com_error err(result);
                LPCTSTR errMsg = err.ErrorMessage();
                MyLog(__FUNCTION__": ", errMsg, "\n");
            }

            // Initialize the overlapped event. This event is freed when the
            // hydration completes and has the same scope as m_overlappedHydration.
            wil::unique_event_nothrow overlappedEvent;
            if (overlappedEvent.try_create(wil::EventOptions::ManualReset, nullptr))
            {
                ZeroMemory(&m_overlappedHydration, sizeof(OVERLAPPED));
                //m_overlappedHydration.hEvent = overlappedEvent.get();
                MyLog(__FUNCTION__": Saving overlappedHydration\n");
            }
            else
            {
                MyLog(__FUNCTION__": !!!Couldn't create overlappedHydration\n");
                _com_error err(result);
                LPCTSTR errMsg = err.ErrorMessage();
                MyLog(__FUNCTION__": ", errMsg, "\n");
            }

            //hydrationRequestVariables.isRequestSuccessful = true;
            //m_map[filePath] = hydrationRequestVariables;
            return true;
        }

        hydrationRequestVariables.isRequestSuccessful = false;
        m_map[filePath] = hydrationRequestVariables;
        return false;
    }

    void MainWindow::CancelHydration(std::wstring_view filePath)
    {
        PrintCancellationOutput(L"=== Starting ===");

        // Look for the filepath in the dictionary
        if (auto it{ m_map.find(filePath) }; it != std::end(m_map))
        {
            PrintCancellationOutput(L"Found hydration variables for filepath");

            auto [key, value] = *it;

            if (value.isRequestSuccessful)
            {
                //if (CancelIoEx(m_placeholder.get(), &m_overlappedHydration))
                if (CancelIoEx(value.PlaceholderHandle.get()->get(), NULL))
                {
                    PrintCancellationOutput(L"=== Cancelled successfully ===\n");
                    return;
                }
                else
                {
                    _com_error err = GetLastError();
                    LPCTSTR errMsg = err.ErrorMessage();
                    PrintCancellationOutput(winrt::to_hstring(errMsg));
                    PrintCancellationOutput(L"CancelIoEx returned false");
                }
            }
            else
            {
                PrintCancellationOutput(L"Hydration request wasn't successful so there's nothing to cancel");
            }
        }
        else
        {
            PrintCancellationOutput(L"Cannot find hydration variables for filepath");
        }

        PrintCancellationOutput(L"=== Failed to cancel ===\n");
    }
}
