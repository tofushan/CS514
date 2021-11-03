from django.shortcuts import render, redirect
from django.contrib import messages
from django.contrib.auth.forms import UserCreationForm
from django.contrib.auth.decorators import login_required
from .forms import UserRegisterForm, DriverRegisterForm, UserUpdateForm, DriverUpdateForm, RideRequestForm, RideUpdateForm
from .forms import DriverFinishForm, RideSearchForm
from .models import Driver, Owner, Sharer, Ride
from django import forms
from django.contrib.auth.models import User
from django.db import IntegrityError
from django.http import HttpResponse, HttpResponseRedirect
from django.core.mail import send_mail, send_mass_mail

def home(request):
    user = User.objects.filter(pk = request.user.id).first()
    if user is not None:
        driver = Driver.objects.filter(user = request.user).first()
        if driver is not None and driver.is_driver is True:
            return render(request, 'users/index.html', {'is_driver': True})
        else:
            return render(request, 'users/index.html', {'is_driver': False})
    else:
        return render(request, 'users/index.html', {'is_driver': False})


def register(request):
    if request.method == 'POST':
        form = UserRegisterForm(request.POST)
        if form.is_valid():
            form.save()
            messages.success(request, f'You are now able to login!')
            return redirect('login')
    else:
        form = UserRegisterForm()
    return render(request, 'users/register.html', {'form': form})


@login_required
def profile(request):
    driver = request.user.driver
    return render(request, 'users/profile.html', {'driver':driver})

#driver update view
@login_required
def driver_update(request):
    if request.method == 'POST':
        user = request.user
        driver_update_form = DriverUpdateForm(request.POST,instance = user.driver)
        if driver_update_form.is_valid():
            driver_update_form.save()
            messages.success(request, f'You have updated your driver info!')
            return redirect('profile')
    else:
        driver_update_form = DriverUpdateForm()

    return render(request, 'users/driver_update.html', {'form':driver_update_form})


#driver register view
@login_required
def driver_register(request):
    driver_register_form = DriverRegisterForm()
    if request.method == 'POST':
        driver = Driver.objects.filter(user = request.user).first()
        if driver is not None and driver.is_driver is True:
            messages.error(request, 'You are already a driver')
            return render(request, 'users/driver_register.html', {'form':driver_register_form}) 
        else:
            driver_register_form = DriverRegisterForm(request.POST, instance=driver)
            if driver_register_form.is_valid():
                driver = driver_register_form.save(commit=False)
                driver.user = request.user
                driver.is_driver = True
                driver_register_form.save()
                return redirect('home-page')
    return render(request, 'users/driver_register.html' , {'form':driver_register_form})
    

#ride status view
def ride_status(request):
    user = User.objects.filter(pk = request.user.id).first()
    if user is not None:
        driver_ride = Ride.objects.filter(driver = request.user).exclude(status='Complete').first()
        owner_ride = Ride.objects.filter(owner = request.user).exclude(status='Complete').first()
        sharer_ride = Ride.objects.filter(sharer = request.user).exclude(status='Complete').first()
        if driver_ride is not None:
            return render(request, 'ride/ride_status.html', {'ride':driver_ride})
        elif owner_ride is not None:
            return render(request, 'ride/ride_status.html', {'ride':owner_ride})
        elif sharer_ride is not None:
            return render(request, 'ride/ride_status.html', {'ride':sharer_ride})
        else:
            messages.error(request, f'You do not have any ongoing rides!')
            return redirect('home-page')
    else:
        messages.error(request, f'You do not have any ongoing rides!')
        return redirect('home-page')


#ride request view
@login_required
def ride_request(request):
    ride_request_form = RideRequestForm()
    obj = Ride.objects.filter(owner = request.user).first()
    if request.method == 'POST':
        if obj is not None:
            messages.error(request, f'Ride with this owner already existed!')
            return redirect('ride-request')
        else:
            ride_request_form = RideRequestForm(request.POST)
            if ride_request_form.is_valid():
                ride = ride_request_form.save(commit = False)
                ride.destination = ride_request_form.cleaned_data['destination']
                ride.owner = str(request.user)
                ride.status = 'Open'
                ride_request_form.save()
                messages.success(request, f'You have requested a new ride!')
                return redirect('ride-status')
        
    return render(request, 'ride/ride_request.html', {'form': ride_request_form})


#ride info update view
@login_required
def ride_update(request):
    ride_update_form = RideUpdateForm()
    ride = Ride.objects.filter(owner = str(request.user), status='Open').first()
    if ride is None:
        messages.error(request, f'You do not have any ongoing rides!')
        return redirect('home-page')

    if request.method == 'POST':
        ride_update_form = RideUpdateForm(request.POST, instance=ride)
        if ride_update_form.is_valid():
            ride_update_form.save()
            messages.success(request, f'You have updated the rides!')
            return redirect('ride-status')
    return render(request, 'ride/ride_update.html',  {'form':ride_update_form})



#for driver and sharer to search
def ride_search(request):
    rides = Ride.objects.filter(status='Open').exclude(owner = None).exclude(owner=str(request.user))
    if rides is None or not rides:
        messages.error(request, f'No open rides so far')
        return redirect('home-page')
    elif request.method == 'POST':
        if 'claim_the_ride' in request.POST:
            driver = Driver.objects.filter(user = request.user).first() 
            if driver is not None:
                cur_ride = Ride.objects.filter(status='Open').exclude(owner = None).exclude(owner=str(request.user)).first()
                if cur_ride is None:
                    messages.error(request, f'You cannot take this ride')
                    return render(request, 'ride/ride_search.html', {'form':None, 'rides':rides})
                elif cur_ride.total_pass > driver.max_pass:
                    messages.error(request, f'You cannot take this ride')
                    return render(request, 'ride/ride_search.html', {'form':None, 'rides':rides})
                
                messages.success(request, f'You have successfully taken the ride')
                cur_ride.driver = str(request.user)
                cur_ride.status = 'Comfirmed'
                cur_ride.save()
                driver_email = User.objects.filter(username=request.user.username).first().email
                owner_email = User.objects.filter(username=cur_ride.owner).first().email
                sharer_email = 'jojo@jo.com'
                if cur_ride.sharer is not None:
                    sharer_email = User.objects.filter(username=str(cur_ride.sharer)).first().email
                datatuple = (
                    ('Ride status changed', 'A driver has confirmed your ride', driver_email, [driver_email]),
                    ('Ride status changed', 'A driver has confirmed your ride', driver_email, [owner_email]),
                    ('Ride status changed', 'A driver has confirmed your ride', driver_email, [sharer_email]),
                )
                send_mass_mail(datatuple)
                print(driver_email)
                print(owner_email)
                print(sharer_email)
                return redirect('ride-status')
            #else the user must be a sharer
            else:
                cur_ride = Ride.objects.filter(sharing=True).exclude(owner=str(request.user)).exclude(owner=None).exclude(driver=str(request.user)).first()
                if cur_ride is None or not cur_ride:
                    messages.error(request, f'You cannot take this ride')
                    return render(request, 'ride/ride_search.html', {'form':None, 'rides':rides})
                else:
                    messages.success(request, f'You have successfully join the ride')
                    cur_ride.total_pass += 1
                    cur_ride.sharer = str(request.user)
                    cur_ride.save()
                    return redirect('ride-status')
        else:
            #driver just entered ride searching info
            form = RideSearchForm(request.POST)
            if form.is_valid():
                dst = form.cleaned_data['destination']
                vehicle_type = form.cleaned_data['vehicle_type']
                rides = Ride.objects.filter(status='Open',destination=dst,vehicle_type=vehicle_type)
                print(rides)
                if rides is None or not rides:
                    messages.error(request, f'No rides fit your requirement')
                    form = RideSearchForm()
                    return render(request, 'ride/ride_search.html', {'form':form, 'rides':None})
                else:
                    return render(request, 'ride/ride_search.html', {'form':None, 'rides':rides})
            else:
                messages.error(request, f'Please re-enter info')
                form = RideSearchForm()
                return render(request, 'ride/ride_search.html', {'form':form, 'rides':None})
    else:
        form = RideSearchForm()
        return render(request, 'ride/ride_search.html',  {'form':form, 'rides':None})


#for driver to complete a ride
def ride_finish(request):
    ride = Ride.objects.filter(driver = str(request.user)).exclude(status='Complete').first()
    if ride is None:
        messages.error(request ,f'You do not have ongoing rides')
        return redirect('home-page')

    elif 'ride_finish' in request.POST:
        ride.status = 'Complete'
        ride.delete()
        messages.success(request, f'Cong! You have finished one ride!')
        return redirect('ride-status')
    else:
        return render(request, 'ride/ride_finish.html')
    