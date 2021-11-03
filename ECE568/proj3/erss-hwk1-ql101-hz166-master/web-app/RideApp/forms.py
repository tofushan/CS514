from django import forms
from django.contrib.auth.models import User
from django.contrib.auth.forms import UserCreationForm
from .models import RideUser


# class UserRegisterForm(UserCreationForm):
#     email = forms.EmailField()
#
#     class Meta:
#         model = User
#         fields = ['username', 'email', 'password1', 'password2']
#
#         # fields = ['username','email','password1',"is_driver","vehicle_type", "license_plate_number", "max_car_pass",
#         # "special_request"]

class UserRegisterForm(UserCreationForm):
    email = forms.EmailField()

    class Meta:
        model = User
        fields = ['username', 'email', 'password1', 'password2']

        # fields = ['username','email','password1',"is_driver","vehicle_type", "license_plate_number", "max_car_pass",
        # "special_request"]


class DriverSearchForm(forms.Form):
    vehicle_capacity = forms.IntegerField()
    vehicle_type = forms.CharField(initial='null', help_text='if no special requirement, please stay null')
    special_request = forms.CharField(initial='null', help_text='if no special requirement, please stay null')


# class DriverUpdateForm(forms.Form):
#     is_driver = forms.BooleanField()
#     vehicle_type = forms.CharField(initial='null', help_text="user's vehicle type")
#     special_request = forms.CharField(initial='null', help_text="user's special request")
#     max_car_pass = forms.IntegerField(help_text="car's max passenger")
#     license_plate_number = forms.CharField(help_text="user's license_plate_number")

class DriverUpdateForm(forms.ModelForm):
    class Meta:
        model = RideUser
        fields = ('is_driver','vehicle_type','special_request','max_car_pass','license_plate_number')


class SharerSearchForm(forms.Form):
    dst = forms.CharField()
    early_time = forms.DateTimeField(help_text='Format: 2020-01-01 12:00')
    last_time = forms.DateTimeField(help_text='Format: 2020-01-01 12:00')
    share_pass_num = forms.IntegerField()

