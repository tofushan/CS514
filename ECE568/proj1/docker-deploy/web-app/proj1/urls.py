"""proj1 URL Configuration

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
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
from django.contrib import admin
from django.contrib.auth import views as auth_views
from django.urls import path, include
from django.conf import settings
from django.conf.urls.static import static
from users import views as user_views

urlpatterns = [
    path('admin/', admin.site.urls),
    path('register/', user_views.register, name = 'register'),
    path('profile/', user_views.profile, name='profile'),
    path('driver_register/', user_views.driver_register, name='driver-register'),
    path('driver_update/', user_views.driver_update, name='driver-update'),
    path('ride_request/', user_views.ride_request, name='ride-request'),
    path('ride_status/', user_views.ride_status, name='ride-status'),
    path('ride_finish/', user_views.ride_finish, name='ride-finish'),
    path('ride_search/', user_views.ride_search, name='ride-search'),
    path('ride_update/', user_views.ride_update, name='ride-update'),
    path('login/', auth_views.LoginView.as_view(template_name = 'users/login.html'), name = 'login'),
    path('logout/', auth_views.LogoutView.as_view(template_name = 'users/logout.html'), name = 'logout'),
    path('', user_views.home, name='home-page'),
]
