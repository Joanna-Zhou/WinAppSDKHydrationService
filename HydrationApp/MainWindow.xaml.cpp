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
        StartButton().IsEnabled(false);

        co_await HydrateFileAsync(FilePath().Text());

        StartButton().IsEnabled(true);

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
        hydrationRequestVariables.PlaceholderHandle.reset(CreateFile(filePath.data(), 0, FILE_SHARE_VALID_FLAGS, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr));

        if (hydrationRequestVariables.PlaceholderHandle.get() == INVALID_HANDLE_VALUE)
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

            auto result = CfHydratePlaceholder(hydrationRequestVariables.PlaceholderHandle.get(), offset, lengthOfEntireFile, CF_HYDRATE_FLAG_NONE, &(hydrationRequestVariables.OverlappedHydration));

            if (!SUCCEEDED(result))
            {
                MyLog(__FUNCTION__": !!!Couldn't send result\n");
                _com_error err(result);
                LPCTSTR errMsg = err.ErrorMessage();
                MyLog(__FUNCTION__": ", errMsg, "\n");
            }
            else
            {
                // Initialize the overlapped event. This event is freed when the
                // hydration completes and has the same scope as m_overlappedHydration.
                wil::unique_event_nothrow overlappedEvent;
                if (overlappedEvent.try_create(wil::EventOptions::ManualReset, nullptr))
                {
                    ZeroMemory(&(hydrationRequestVariables.OverlappedHydration), sizeof(OVERLAPPED));
                    hydrationRequestVariables.OverlappedHydration.hEvent = overlappedEvent.get();
                    MyLog(__FUNCTION__": Saving overlappedHydration\n");
                }
                else
                {
                    MyLog(__FUNCTION__": !!!Couldn't create overlappedHydration\n");
                    _com_error err(result);
                    LPCTSTR errMsg = err.ErrorMessage();
                    MyLog(__FUNCTION__": ", errMsg, "\n");
                }

                hydrationRequestVariables.isRequestSuccessful = true;
                return true;
            }
        }

        hydrationRequestVariables.isRequestSuccessful = false;
        m_map.insert({ filePath, hydrationRequestVariables });
        return false;
    }

    void MainWindow::CancelHydration(std::wstring_view filePath)
    {
        PrintCancellationOutput(L"=== Starting ===");

        // Look for the filepath in the dictionary
        if (auto it{ m_map.find(filePath)}; it != std::end(m_map))
        {
            PrintCancellationOutput(L"Found hydration variables for filepath");

            auto [key, value] = *it;

            //if (value.isRequestSuccessful)
            //{
                if (CancelIoEx(value.PlaceholderHandle.get(), &(value.OverlappedHydration)))
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
            //}
            //else
            //{
            //    PrintCancellationOutput(L"Hydration request wasn't successful so there's nothing to cancel");
            //}
        }
        else
        {
            PrintCancellationOutput(L"Cannot find hydration variables for filepath");
        }
        
        PrintCancellationOutput(L"=== Failed to cancel ===\n");
    }
}
