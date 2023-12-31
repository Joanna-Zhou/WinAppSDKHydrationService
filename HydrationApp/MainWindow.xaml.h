// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once

#include "MainWindow.g.h"
#include <wil/resource.h>
#include <map>

namespace winrt::HydrationApp::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        winrt::hstring HydrationOutputText() { return m_hydrationOutputText; };
        void HydrationOutputText(winrt::hstring v) { m_hydrationOutputText = v; m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ L"HydrationOutputText" });};

        winrt::hstring CancellationOutputText() { return m_cancellationOutputText; };
        void CancellationOutputText(winrt::hstring v) { m_cancellationOutputText = v; m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ L"CancellationOutputText" }); };

        Windows::Foundation::IAsyncAction StartButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void CancelButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);

        winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler) { return m_propertyChanged.add(handler); };
        void PropertyChanged(winrt::event_token const& token) { m_propertyChanged.remove(token); };

    private:
        Windows::Foundation::IAsyncAction HydrateFileAsync(std::wstring_view filePath);
        bool HydrateFile(std::wstring_view filePath);
        void CancelHydration(std::wstring_view filePath);

        inline void PrintHydrationOutput(winrt::hstring newLine) { HydrationOutputText(m_hydrationOutputText + L"\n" + newLine); }
        inline void PrintCancellationOutput(winrt::hstring newLine) { CancellationOutputText(m_cancellationOutputText + L"\n" + newLine); }

        struct HydrationRequestVariables
        {
            std::shared_ptr<winrt::handle> PlaceholderHandle{};
            bool isRequestSuccessful = false;
        };

        std::map<std::wstring_view, HydrationRequestVariables> m_map;
        OVERLAPPED m_overlappedHydration{};
        wil::unique_hfile m_placeholder;

        winrt::hstring m_hydrationOutputText = L"";
        winrt::hstring m_cancellationOutputText = L"";
        
        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

        winrt::apartment_context m_uiThread;
    };
}

namespace winrt::HydrationApp::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
