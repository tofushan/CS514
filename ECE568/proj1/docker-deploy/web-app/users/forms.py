from django import forms
from django.contrib.auth.models import User
from django.contrib.auth.forms import UserCreationForm
from .models import Driver, Owner, Sharer, Ride

class RideRequestForm(forms.ModelForm):
    class Meta:
        model = Ride
        fields = ['destination', 'arrival_time', 'total_pass', 'sharing', 'vehicle_type', 'other_request']

class RideUpdateForm(forms.ModelForm):
    class Meta:
        model = Ride
        fields = ['destination', 'arrival_time', 'total_pass', 'sharing', 'vehicle_type', 'other_request']

class UserRegisterForm(UserCreationForm):
    class Meta:
        model = User
        fields = ['username', 'email', 'password1', 'password2']

class UserUpdateForm(forms.ModelForm):
    email = forms.EmailField()
    class Meta:
        model = User
        fields = ['username', 'email']


class DriverRegisterForm(forms.ModelForm):
    class Meta:
        model = Driver
        fields = ['plate_number', 'max_pass', 'vehicle_type']


class DriverUpdateForm(forms.ModelForm):
    class Meta:
        model = Driver
        fields = ['plate_number', 'max_pass', 'vehicle_type']

class DriverFinishForm(forms.ModelForm):
    class Meta:
        model = Ride
        fields = ['status']

class RideSearchForm(forms.ModelForm):
    class Meta:
        model = Ride
        fields = ['destination', 'arrival_time', 'vehicle_type', 'total_pass']