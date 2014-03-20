using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using System.Windows.Controls.Primitives;
using PhoneDirect3DXamlAppComponent;
using System.Windows.Media;
using PhoneDirect3DXamlAppInterop.Resources;

namespace PhoneDirect3DXamlAppInterop
{
    enum ConnectionState
    {
        LINK_OK,
        LINK_ERROR,
        LINK_NEEDS_UPDATE,
        LINK_ABORT
    };

    public partial class StartLinkControl : UserControl
    {
        private bool initdone = false;

        public static Direct3DBackground m_d3dBackground;
        public StartLinkControl()
        {
            InitializeComponent();

            txtAddress.Text = App.metroSettings.LastIPAddress ;
            txtTimeout.Text = App.metroSettings.LastTimeout.ToString();

            this.Loaded += (o, e) =>
            {
                initdone = true;
            };
        }

        private void Cancelbtn_Click(object sender, RoutedEventArgs e)
        {
            m_d3dBackground.StopConnectLoop();
            ClosePopup();
        }

        private void ClosePopup()
        {

            Popup selectPop = this.Parent as Popup;
            selectPop.IsOpen = false;

        }

        private async void ConnectButton_Click(object sender, RoutedEventArgs e)
        {
            //save setting
            App.metroSettings.LastIPAddress = txtAddress.Text;
            App.metroSettings.LastTimeout = int.Parse(txtTimeout.Text);

            if (rolePicker.SelectedIndex == 0) //server
            {
                ConnectButton.IsEnabled = false;

                String ipaddress = m_d3dBackground.SetupSocket(true, 2, int.Parse(txtTimeout.Text), "");
                tblkStatus.Foreground = (SolidColorBrush)App.Current.Resources["PhoneForegroundBrush"];
                tblkStatus.Text = String.Format(AppResources.HostIPAddressText, ipaddress);


                var asyncAction = m_d3dBackground.ConnectSocket();


                await asyncAction;

                if ((ConnectionState)asyncAction.GetResults() == ConnectionState.LINK_OK)
                {
                    tblkStatus.Foreground = new SolidColorBrush(Colors.Green);
                    tblkStatus.Text = AppResources.LinkConnectedText;
                }
                else
                {
                    new SolidColorBrush(Colors.Red);
                    tblkStatus.Text = AppResources.LinkErrorText;
                }

                ConnectButton.IsEnabled = true;


            }
            else //client
            {
                ConnectButton.IsEnabled = false;

                String ipaddress = m_d3dBackground.SetupSocket(false, 2, int.Parse(txtTimeout.Text), txtAddress.Text);

                var asyncAction = m_d3dBackground.ConnectSocket();

                await asyncAction;

                if ((ConnectionState)asyncAction.GetResults() == ConnectionState.LINK_OK)
                {
                    tblkStatus.Foreground = new SolidColorBrush(Colors.Green);
                    tblkStatus.Text = AppResources.LinkConnectedText;
                }
                else
                {
                    new SolidColorBrush(Colors.Red);
                    tblkStatus.Text = AppResources.LinkErrorText;
                }

                ConnectButton.IsEnabled = true;


            }
        }

        private void rolePicker_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (initdone)
            {
                if (rolePicker.SelectedIndex == 0) //server
                {
                    txtAddress.Visibility = Visibility.Collapsed;
                    tblkStatus.Text = AppResources.TapStartServerIPText;
                    ConnectButton.Content = AppResources.StartServerText;
                }
                else
                {
                    txtAddress.Visibility = Visibility.Visible;
                    tblkStatus.Text = AppResources.StartServerOtherText;
                    ConnectButton.Content = AppResources.ConnectText;
                }
            }
        }

       
    }
}
