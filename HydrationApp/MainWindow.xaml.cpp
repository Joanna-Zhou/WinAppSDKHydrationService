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
        StartButton().Content(box_value(L"Hydrating"));

        HydrateFile(FilePath().Text());

        StartButton().Content(box_value(L"Start Hydration"));
    }

    void MainWindow::CancelButton_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        CancelButton().Content(box_value(L"Cancelling"));

        //CancelButton().Content(box_value(L"Clicked"));

        CancelButton().Content(box_value(L"Cancelling"));
    }

    void MainWindow::HydrateFile(std::wstring_view filePath)
    {
        PrintOutputLine(L"=== Starting ===");
        m_isHydrated = false;

        winrt::handle placeholder(CreateFile(filePath.data(), 0, FILE_READ_DATA, nullptr, OPEN_EXISTING, 0, nullptr));
        //m_placeholder.reset(CreateFile(filePath.data(), 0, FILE_SHARE_VALID_FLAGS, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr));

        if (placeholder.get() == INVALID_HANDLE_VALUE)
        {
            _com_error err = GetLastError();
            LPCTSTR errMsg = err.ErrorMessage();
            PrintOutputLine(winrt::to_hstring(errMsg));
        }
        else
        {
            PrintOutputLine(L"Successfully got placeholder...");
            LARGE_INTEGER offset;
            offset.QuadPart = 0;
            LARGE_INTEGER lengthOfEntireFile;
            lengthOfEntireFile.QuadPart = -1;

            PrintOutputLine(L"Sending request...");
            auto result = CfHydratePlaceholder(placeholder.get(), offset, lengthOfEntireFile, CF_HYDRATE_FLAG_NONE, NULL);

            //PrintOutputLine(L"HydrateFile: getting result...");

            //m_placeholder.close();

            _com_error err(result);
            LPCTSTR errMsg = err.ErrorMessage();
            PrintOutputLine(winrt::to_hstring(errMsg));

            if (SUCCEEDED(result))
            {
                PrintOutputLine(L"=== Successful ===\n");
                m_isHydrated = true;
                return;
            }
        }

        PrintOutputLine(L"=== Failed ===\n");

        return;
    }

    //WF::IAsyncOperation<bool> MainWindow::HydrateFileAsync(std::wstring_view filePath)
    //{
    //    /* Resume on a background thread here since HydrateFile()
    //     * will block the calling thread (ie. it will cause UI to hang). */
    //     // No need for get_strong because this is a free function
    //    co_await winrt::resume_background();

    //    co_return HydrateFile(filePath);
    //}
}
