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

    void MainWindow::StartButton_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        StartButton().IsEnabled(false);

        HydrateFile(FilePath().Text());

        StartButton().IsEnabled(true);
    }

    void MainWindow::CancelButton_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        CancelButton().IsEnabled(false);

        CancelHydration();

        CancelButton().IsEnabled(true);
    }

    void MainWindow::HydrateFile(std::wstring_view filePath)
    {
        PrintHydrationOutput(L"=== Starting ===");
        m_isHydrated = false;

        //winrt::handle placeholder(CreateFile(filePath.data(), 0, FILE_READ_DATA, nullptr, OPEN_EXISTING, 0, nullptr));
        m_placeholder.reset(CreateFile(filePath.data(), 0, FILE_SHARE_VALID_FLAGS, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr));

        if (m_placeholder.get() == INVALID_HANDLE_VALUE)
        {
            _com_error err = GetLastError();
            LPCTSTR errMsg = err.ErrorMessage();
            PrintHydrationOutput(winrt::to_hstring(errMsg));
        }
        else
        {
            PrintHydrationOutput(L"Successfully got placeholder");
            LARGE_INTEGER offset;
            offset.QuadPart = 0;
            LARGE_INTEGER lengthOfEntireFile;
            lengthOfEntireFile.QuadPart = -1;

            PrintHydrationOutput(L"Sending request");
            auto result = CfHydratePlaceholder(m_placeholder.get(), offset, lengthOfEntireFile, CF_HYDRATE_FLAG_NONE, NULL);

            // Initialize the overlapped event. This event is freed when the
            // hydration completes and has the same scope as m_overlappedHydration.
            wil::unique_event_nothrow overlappedEvent;
            if (overlappedEvent.try_create(wil::EventOptions::ManualReset, nullptr))
            {
                ZeroMemory(&m_overlappedHydration, sizeof(OVERLAPPED));
                m_overlappedHydration.hEvent = overlappedEvent.get();
                PrintHydrationOutput(L"Saving overlappedHydration");
            }
            else
            {
                PrintHydrationOutput(L"!!!Couldn't create overlappedHydration");
            }

            _com_error err(result);
            LPCTSTR errMsg = err.ErrorMessage();
            PrintHydrationOutput(winrt::to_hstring(errMsg));

            if (SUCCEEDED(result))
            {
                PrintHydrationOutput(L"=== Successful ===\n");
                m_isHydrated = true;
                return;
            }
        }

        PrintHydrationOutput(L"=== Failed ===\n");

        return;
    }

    void MainWindow::CancelHydration()
    {
        PrintCancellationOutput(L"=== Starting ===");

        if (CancelIoEx(m_placeholder.get(), &m_overlappedHydration))
        {
            PrintHydrationOutput(L"=== Successful ===\n");
        }
        else
        {
            _com_error err = GetLastError();
            LPCTSTR errMsg = err.ErrorMessage();
            PrintCancellationOutput(winrt::to_hstring(errMsg));
        }

        //RETURN_LAST_ERROR_IF_FALSE(CancelIoEx(m_placeholder.get(), &m_overlappedHydration));

        PrintCancellationOutput(L"=== Failed ===\n");
    }
}
