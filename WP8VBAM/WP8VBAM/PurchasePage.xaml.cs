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

using System.Collections.ObjectModel;

using System.Threading.Tasks;

using Windows.ApplicationModel.Store;
using Store = Windows.ApplicationModel.Store;
using PhoneDirect3DXamlAppComponent;

using PhoneDirect3DXamlAppInterop.Resources;




namespace PhoneDirect3DXamlAppInterop
{
    public partial class PurchasePage : PhoneApplicationPage
    {
        public PurchasePage()
        {

            InitializeComponent();

#if GBC
            SystemTray.GetProgressIndicator(this).Text = AppResources.ApplicationTitle2;
#endif

            pics.ItemsSource = picItems;
        }

        public ObservableCollection<ProductItem> picItems = new ObservableCollection<ProductItem>();

        protected override void OnNavigatedTo(System.Windows.Navigation.NavigationEventArgs e)
        {
            RenderStoreItems();
            base.OnNavigatedTo(e);
        }

        private async Task RenderStoreItems()
        {
            picItems.Clear();

            try
            {

                //StoreManager mySM = new StoreManager();
                ListingInformation li = await Store.CurrentApp.LoadListingInformationAsync();
                txtError.Visibility = Visibility.Collapsed; //if error, it would go to catch.

                string key = "";
                ProductListing pListing = null;
                string imageLink = "";
                string status = "";
                string pname = "";
                Visibility buyButtonVisibility = Visibility.Collapsed;


                // no ads
                key = "removeads";
                imageLink = "/Assets/Icons/noad_icon.png";
                if (li.ProductListings.TryGetValue(key, out pListing))
                {
                    ProductLicense license = Store.CurrentApp.LicenseInformation.ProductLicenses[key];
                    status = license.IsActive ? "Purchased, thank you!" : pListing.FormattedPrice;
                    //string receipt = await Store.CurrentApp.GetProductReceiptAsync(license.ProductId);

                    buyButtonVisibility = Store.CurrentApp.LicenseInformation.ProductLicenses[key].IsActive ? Visibility.Collapsed : Visibility.Visible;
                    pname = pListing.Name;
                }
                else
                {
                    status = "Product is in certification with MS. Please try again in tomorrow.";
                    buyButtonVisibility = Visibility.Collapsed;
                    pname = "Remove ads";
                }

                picItems.Add(
                    new ProductItem
                    {
                        imgLink = imageLink,
                        Name = pname,
                        Status = status,
                        key = key,
                        BuyNowButtonVisible = buyButtonVisibility
                    }
                );


                // get silver in-app purcase
                key = "premiumfeatures";
                imageLink = "/Assets/Icons/plus_sign.png";
                if (li.ProductListings.TryGetValue(key, out pListing))
                {
                    status = Store.CurrentApp.LicenseInformation.ProductLicenses[key].IsActive ? "Purchased, thank you!" : pListing.FormattedPrice;
                    buyButtonVisibility = Store.CurrentApp.LicenseInformation.ProductLicenses[key].IsActive ? Visibility.Collapsed : Visibility.Visible;
                    pname = pListing.Name;
                }
                else
                {
                    status = "Product is in certification with MS. Please try again in tomorrow.";
                    buyButtonVisibility = Visibility.Collapsed;
                    pname = "Premium Features";
                }

                picItems.Add(
                    new ProductItem
                    {
                        imgLink = imageLink,
                        Name = pname,
                        Status = status,
                        key = key,
                        BuyNowButtonVisible = buyButtonVisibility
                    }
                );

                // get gold in-app purcase
                key = "noads_premium";
                imageLink = "/Assets/Icons/noad_plus_icon.png";
                if (li.ProductListings.TryGetValue(key, out pListing))
                {
                    status = Store.CurrentApp.LicenseInformation.ProductLicenses[key].IsActive ? "Purchased, thank you!" : pListing.FormattedPrice;
                    buyButtonVisibility = Store.CurrentApp.LicenseInformation.ProductLicenses[key].IsActive ? Visibility.Collapsed : Visibility.Visible;
                    pname = pListing.Name;
                }
                else
                {
                    status = "Product is in certification with MS. Please try again in tomorrow.";
                    buyButtonVisibility = Visibility.Collapsed;
                    pname = "No Ads + Premium Features";
                }

                picItems.Add(
                    new ProductItem
                    {
                        imgLink = imageLink,
                        Name = pname,
                        Status = status,
                        key = key,
                        BuyNowButtonVisible = buyButtonVisibility
                    }
                );




            }
            catch (Exception e)
            {
                System.Diagnostics.Debug.WriteLine(e.ToString());
                txtError.Visibility = Visibility.Visible;
            }
            finally
            {
                txtLoading.Visibility = Visibility.Collapsed;
            }
        } //end renderstoresitem

        private async void ButtonBuyNow_Clicked(object sender, RoutedEventArgs e)
        {
            Button btn = sender as Button;

            string key = btn.Tag.ToString();


            if (!Store.CurrentApp.LicenseInformation.ProductLicenses[key].IsActive)
            {
                ListingInformation li = await Store.CurrentApp.LoadListingInformationAsync();
                string pID = li.ProductListings[key].ProductId;

                try
                {
                    string receipt = await Store.CurrentApp.RequestProductPurchaseAsync(pID, false);

                    //reread license
                    App.DetermineIsTrail();

                    //prompt user to restart app if it's ad removal
                    if (key == "removeads" || key == "noads_premium")
                        System.Windows.Deployment.Current.Dispatcher.BeginInvoke(() =>
                        {
                            MessageBox.Show("Purchase successful, ads will not be shown the next time you start the app.");
                        });

                    if (key == "premiumfeatures")
                        System.Windows.Deployment.Current.Dispatcher.BeginInvoke(() =>
                        {
                            MessageBox.Show("Purchase successful.");
                        });

                    //enable moga controller (in case user does not know that he has to enable it)
                    //if (key == "premiumfeatures" || key == "noads_premium")
                    //    EmulatorSettings.Current.UseMogaController = true;
                }
                catch (Exception)
                { }
            }

            
        }

        private void TextBlock_Tap(object sender, System.Windows.Input.GestureEventArgs e)
        {
            MarketplaceDetailTask marketplaceDetailTask = new MarketplaceDetailTask();
            marketplaceDetailTask.ContentIdentifier = "ed3cc816-1ab0-418a-9bb8-11505804f6b4";
            marketplaceDetailTask.Show();
        }

        private void TextBlock_Tap_1(object sender, System.Windows.Input.GestureEventArgs e)
        {
            WebBrowserTask wbtask = new WebBrowserTask();
            wbtask.Uri = new Uri("http://www.youtube.com/watch?v=YfqzZhcr__o");
            wbtask.Show();
        }


    }


    public class ProductItem
    {
        public string imgLink { get; set; }
        public string Status { get; set; }
        public string Name { get; set; }
        public string key { get; set; }
        public Visibility BuyNowButtonVisible { get; set; }
    }
}