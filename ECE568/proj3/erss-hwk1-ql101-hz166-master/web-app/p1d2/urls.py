"""p1d2 URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/3.0/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path

"""
from django.contrib import admin
from django.urls import include, path
from RideApp import views as rideapp_views
from django.contrib.auth import views as auth_views
#  127.0.0.1:8000/
urlpatterns = [
    path('admin/', admin.site.urls),
    path('register/',rideapp_views.register, name='register'),
    path('login/',auth_views.LoginView.as_view(template_name='RideApp/login.html'), name='login'),
    path('logout/',auth_views.LogoutView.as_view(template_name='RideApp/logout.html'), name='logout'),
    path('', include('RideApp.urls')),
]
