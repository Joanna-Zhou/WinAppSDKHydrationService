// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

//#include <bcrypt.h>
//#include <cfapi.h>

//#include <winrt/Microsoft.UI.Xaml.Input.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::HydrationApp::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();
    }

    void MainWindow::StartButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        StartButton().Content(box_value(L"Hydrating"));

        HydrateFile(FilePath().Text());

        StartButton().Content(box_value(L"Hydrate file"));
    }

    void MainWindow::CancelButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        StartButton().Content(box_value(L"Cancelling"));

        //CancelButton().Content(box_value(L"Clicked"));

        StartButton().Content(box_value(L"Cancelling"));
    }

    bool MainWindow::HydrateFile(std::wstring_view filePath)
    {
        PrintOutputLine(L"HydrateFile: starting...");
        winrt::handle placeholder(CreateFile(filePath.data(), 0, FILE_READ_DATA, nullptr, OPEN_EXISTING, 0, nullptr));

        if (placeholder.get() != INVALID_HANDLE_VALUE)
        {
            LARGE_INTEGER offset;
            offset.QuadPart = 0;
            LARGE_INTEGER lengthOfEntireFile;
            lengthOfEntireFile.QuadPart = -1;

            //auto result = CfHydratePlaceholder(placeholder.get(), offset, lengthOfEntireFile, CF_HYDRATE_FLAG_NONE, NULL);

            //placeholder.close();

            //return (SUCCEEDED(result));
        }

        PrintOutputLine(L"HydrateFile: failed.");

        return false;
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
