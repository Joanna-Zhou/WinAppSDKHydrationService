// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once

#include "MainWindow.g.h"

//#include <winrt/Microsoft.UI.Xaml.Data.h>
//namespace WUXD = winrt::Microsoft::UI::Xaml::Data;

namespace winrt::HydrationApp::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        winrt::hstring OutputText() { return m_outputText; };
        void OutputText(winrt::hstring v) { m_outputText = v; m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ L"OutputText" });};

        void StartButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void CancelButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);

        bool HydrateFile(std::wstring_view filePath);
        //WF::IAsyncOperation<bool> HydrateFileAsync(std::wstring_view filePath);

        winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler) { return m_propertyChanged.add(handler); };
        void PropertyChanged(winrt::event_token const& token) { m_propertyChanged.remove(token); };

    private:
        winrt::hstring m_filePath = L"C:/Users/yizzho/OneDrive - Microsoft/Pictures/TestingPhotos/Video Projects";
        winrt::hstring m_outputText = L"Placeholder";

        inline void PrintOutputLine(winrt::hstring newLine) { OutputText(m_outputText + L"\n" + newLine); }

        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

    };
}

namespace winrt::HydrationApp::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
