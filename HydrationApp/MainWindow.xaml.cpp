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

        //HydrateFile(FilePath().Text());
        co_await HydrateFileAsync(FilePath().Text());

        StartButton().IsEnabled(true);

        co_return;
    }

    void MainWindow::CancelButton_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        CancelButton().IsEnabled(false);

        CancelHydration();

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

        m_isHydrated = false;

        //winrt::handle placeholder(CreateFile(filePath.data(), 0, FILE_READ_DATA, nullptr, OPEN_EXISTING, 0, nullptr));
        m_placeholder.reset(CreateFile(filePath.data(), 0, FILE_SHARE_VALID_FLAGS, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr));

        if (m_placeholder.get() == INVALID_HANDLE_VALUE)
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

            auto result = CfHydratePlaceholder(m_placeholder.get(), offset, lengthOfEntireFile, CF_HYDRATE_FLAG_NONE, &m_overlappedHydration);

            if (!SUCCEEDED(result))
            {
                MyLog(__FUNCTION__": !!!Couldn't send result\n");
                _com_error err(result);
                LPCTSTR errMsg = err.ErrorMessage();
                MyLog(__FUNCTION__": ", errMsg, "\n");
                return false;
            }

            // Initialize the overlapped event. This event is freed when the
            // hydration completes and has the same scope as m_overlappedHydration.
            wil::unique_event_nothrow overlappedEvent;
            if (overlappedEvent.try_create(wil::EventOptions::ManualReset, nullptr))
            {
                ZeroMemory(&m_overlappedHydration, sizeof(OVERLAPPED));
                m_overlappedHydration.hEvent = overlappedEvent.get();
                MyLog(__FUNCTION__": Saving overlappedHydration\n");
            }
            else
            {
                MyLog(__FUNCTION__": !!!Couldn't create overlappedHydration\n");
                _com_error err(result);
                LPCTSTR errMsg = err.ErrorMessage();
                MyLog(__FUNCTION__": ", errMsg, "\n");
            }

            return true;
        }

        return false;
    }

    void MainWindow::CancelHydration()
    {
        PrintCancellationOutput(L"=== Starting ===");

        if (CancelIoEx(m_placeholder.get(), &m_overlappedHydration))
        {
            PrintCancellationOutput(L"=== Cancelled successfully ===\n");
        }
        else
        {
            _com_error err = GetLastError();
            LPCTSTR errMsg = err.ErrorMessage();
            PrintCancellationOutput(winrt::to_hstring(errMsg));
            PrintCancellationOutput(L"=== Failed to cancel ===\n");
        }
    }
}
