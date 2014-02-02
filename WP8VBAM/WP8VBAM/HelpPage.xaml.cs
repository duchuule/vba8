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
    public partial class HelpPage : PhoneApplicationPage
    {
        public HelpPage()
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
            importSkydriveText.Text = AppResources.HelpImportSkyDriveText2;
            importEmailText.Text = AppResources.HelpImportEmailText2;
            importWebText.Text = AppResources.HelpImportWebText2;
            contactBlock.Text = AppResources.AboutContact2;
            SystemTray.GetProgressIndicator(this).Text = AppResources.ApplicationTitle2;
#endif
        }


        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            string strindex = null;
            NavigationContext.QueryString.TryGetValue("index", out strindex);

            if (strindex != null)
                MainPivotControl.SelectedIndex = int.Parse(strindex);

            
            base.OnNavigatedTo(e);
        }

        private void contactBlock_Tap_1(object sender, System.Windows.Input.GestureEventArgs e)
        {
            EmailComposeTask emailcomposer = new EmailComposeTask();
#if !GBC
            emailcomposer.To = AppResources.AboutContact;
#else
            emailcomposer.To = AppResources.AboutContact2;
#endif
            emailcomposer.Subject = AppResources.EmailSubjectText;
            emailcomposer.Body = AppResources.EmailBodyText;
            emailcomposer.Show();
        }
    }
}