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
using PhoneDirect3DXamlAppInterop.Database;
using Microsoft.Phone.Tasks;

namespace PhoneDirect3DXamlAppInterop
{
    public partial class ContextPage : PhoneApplicationPage
    {
        ROMDBEntry entry;

        public ContextPage()
        {
            InitializeComponent();

            this.entry = PhoneApplicationService.Current.State["parameter"] as ROMDBEntry;
            PhoneApplicationService.Current.State.Remove("parameter");

            this.titleBox.Text = this.entry.DisplayName;
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            if (e.NavigationMode == NavigationMode.Back)
            {
                this.NavigationService.GoBack();
            }
        }

        private void cheatBlock_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            if (!App.IsTrial)
            {
                PhoneApplicationService.Current.State["parameter"] = this.entry;
                this.NavigationService.Navigate(new Uri("/CheatPage.xaml", UriKind.Relative));
            }
            else
            {
                ShowBuyDialog();
            }
        }

        private void deleteManageBlock_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            PhoneApplicationService.Current.State["parameter"] = this.entry;
            this.NavigationService.Navigate(new Uri("/ManageSavestatePage.xaml", UriKind.Relative));
        }

        private async void deleteSavesBlock_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            await FileHandler.DeleteSRAMFile(this.entry);

            MessageBox.Show(AppResources.SRAMDeletedSuccessfully, AppResources.InfoCaption, MessageBoxButton.OK);
        }

        private async void deleteBlock_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            try
            {
                await FileHandler.DeleteROMAsync(this.entry);

                this.NavigationService.GoBack();
            }
            catch (System.IO.FileNotFoundException)
            { }
        }

        private void renameBlock_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            PhoneApplicationService.Current.State["parameter"] = this.entry;
            PhoneApplicationService.Current.State["parameter2"] = ROMDatabase.Current;

            this.NavigationService.Navigate(new Uri("/RenamePage.xaml", UriKind.Relative));
        }

        private void pinBlock_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            if (!App.IsTrial)
            {
                try
                {
                    FileHandler.CreateROMTile(this.entry);
                }
                catch (InvalidOperationException)
                {
                    MessageBox.Show(AppResources.MaximumTilesPinned);
                }
            }
            else
            {
                ShowBuyDialog();
            }
        }


        void ShowBuyDialog()
        {
            ShowDialog(AppResources.BuyNowText);
        }

        private static void ShowDialog(String text)
        {
            var result = MessageBox.Show(text, AppResources.InfoCaption, MessageBoxButton.OKCancel);
            if (result == MessageBoxResult.OK)
            {
                MarketplaceDetailTask marketplaceDetailTask = new MarketplaceDetailTask();
                marketplaceDetailTask.ContentType = MarketplaceContentType.Applications;
#if !GBC
                marketplaceDetailTask.ContentIdentifier = "4e3142c4-b99c-4075-bedc-b10a3086327d";
#else
                marketplaceDetailTask.ContentIdentifier = "be33ce3e-e519-4d2c-b30e-83347601ed57";                    
#endif
                marketplaceDetailTask.Show();
            }
        }
    }
}