using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using Microsoft.Phone.Tasks;
using PhoneDirect3DXamlAppInterop.Resources;

namespace PhoneDirect3DXamlAppInterop
{
    public partial class AboutPage : PhoneApplicationPage
    {
        public AboutPage()
        {
            InitializeComponent();

            //create ad control
            if (App.HasAds)
            {
                AdControl adControl = new AdControl();
                ((Grid)(LayoutRoot.Children[0])).Children.Add(adControl);
                adControl.SetValue(Grid.RowProperty, 2);
            }
            tblkVersion.Text = AppResources.AboutVersion + ": " + System.Reflection.Assembly.GetExecutingAssembly()
                    .FullName.Split('=')[1].Split(',')[0]; 

#if GBC
            tblkTitle.Text = AppResources.ApplicationTitle2;
            SystemTray.GetProgressIndicator(this).Text = AppResources.ApplicationTitle2;
#endif
        }

        private void contactBlock_Tap_1(object sender, System.Windows.Input.GestureEventArgs e)
        {
            EmailComposeTask emailcomposer = new EmailComposeTask();

        	emailcomposer.To = AppResources.AboutContact;

            emailcomposer.Subject = AppResources.EmailSubjectText;
            emailcomposer.Body = String.Format(AppResources.EmailBodyText, Microsoft.Phone.Info.DeviceStatus.DeviceName);
        	emailcomposer.Show();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            this.NavigationService.Navigate(new Uri("/LicensePage.xaml", UriKind.Relative));
        }
    }
}