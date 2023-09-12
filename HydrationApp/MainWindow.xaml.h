// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once

#include "MainWindow.g.h"
#include <wil/resource.h>

namespace winrt::HydrationApp::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        winrt::hstring HydrationOutputText() { return m_hydrationOutputText; };
        void HydrationOutputText(winrt::hstring v) { m_hydrationOutputText = v; m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ L"HydrationOutputText" });};

        winrt::hstring CancellationOutputText() { return m_cancellationOutputText; };
        void CancellationOutputText(winrt::hstring v) { m_cancellationOutputText = v; m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ L"CancellationOutputText" }); };

        void StartButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void CancelButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);

        winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler) { return m_propertyChanged.add(handler); };
        void PropertyChanged(winrt::event_token const& token) { m_propertyChanged.remove(token); };

    private:
        void HydrateFile(std::wstring_view filePath);
        void CancelHydration();

        inline void PrintHydrationOutput(winrt::hstring newLine) { HydrationOutputText(m_hydrationOutputText + L"\n" + newLine); }
        inline void PrintCancellationOutput(winrt::hstring newLine) { CancellationOutputText(m_cancellationOutputText + L"\n" + newLine); }
        
        winrt::hstring m_hydrationOutputText = L"";
        winrt::hstring m_cancellationOutputText = L"";
        
        bool m_isHydrated = false;
        wil::unique_hfile m_placeholder;
        OVERLAPPED m_overlappedHydration = {};
        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
    };
}

namespace winrt::HydrationApp::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
