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
        public static Direct3DBackground m_d3dBackground;
        public StartLinkControl()
        {
            InitializeComponent();

            txtAddress.Text = App.metroSettings.LastIPAddress ;
            txtTimeout.Text = App.metroSettings.LastTimeout.ToString();
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
                tblkStatus.Text = "Host's IP address: " + ipaddress + ". Waiting for client..." ;

                var asyncAction = m_d3dBackground.ConnectSocket();


                await asyncAction;

                if ((ConnectionState)asyncAction.GetResults() == ConnectionState.LINK_OK)
                {
                    tblkStatus.Foreground = new SolidColorBrush(Colors.Green);
                    tblkStatus.Text = "Connected. You can now close this window and continue playing game.";
                }
                else
                {
                    new SolidColorBrush(Colors.Red);
                    tblkStatus.Text = "Error occured. Please restart the app and try again.";
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
                    tblkStatus.Text = "Connected. You can now close this window and continue playing game.";
                }
                else
                {
                    new SolidColorBrush(Colors.Red);
                    tblkStatus.Text = "Error occured. Please restart the app and try again.";
                }

                ConnectButton.IsEnabled = true;


            }
        }

        private void rolePicker_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (rolePicker.SelectedIndex == 0) //server
            {
                txtAddress.Visibility = Visibility.Collapsed;
                tblkStatus.Text = "Tap Start server to get the server IP address.";
                ConnectButton.Content = "Start server";
            }
            else
            {
                txtAddress.Visibility = Visibility.Visible;
                tblkStatus.Text = "Start server on the other device to get server's IP address.";
                ConnectButton.Content = "Connect";
            }

        }

       
    }
}
