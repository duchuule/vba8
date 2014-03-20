using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using PhoneDirect3DXamlAppInterop.Resources;
using Microsoft.Phone.Tasks;

namespace PhoneDirect3DXamlAppInterop
{
    public partial class LicensePage : PhoneApplicationPage
    {
        public LicensePage()
        {
            InitializeComponent();

            //create ad control
            if (App.HasAds)
            {
                AdControl adControl = new AdControl();
                LayoutRoot.Children.Add(adControl);
                adControl.SetValue(Grid.RowProperty, 1);
            }

#if GBC
            SystemTray.GetProgressIndicator(this).Text = AppResources.ApplicationTitle2;
#endif
        }

        private void sourceButton_Click(object sender, RoutedEventArgs e)
        {
            WebBrowserTask wbt = new WebBrowserTask();
            wbt.URL = "https://skydrive.live.com/?cid=631eb30445b47731&id=631EB30445B47731%21107&ithint=folder,.zip&authkey=!AMcy8hX76lpiQaw";
            wbt.Show();
        }

        private void gplButton_Click(object sender, RoutedEventArgs e)
        {
            WebBrowserTask wbt = new WebBrowserTask();
            wbt.URL = AppResources.GPLv3;
            wbt.Show();
        }
    }
}