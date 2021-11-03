from django.db import models
from django.contrib.auth.models import User
import datetime
import django.utils

# Create your models here.

type_list = (
    ('suv','SUV'), 
    ('track', 'TRACK'),
    ('sedan', 'SEDAN'),
)

class Driver(models.Model):
    user = models.OneToOneField(User, on_delete = models.CASCADE)
    is_driver = models.BooleanField(default = False)
    plate_number = models.CharField(max_length=100)
    max_pass = models.PositiveIntegerField(default = 5)
    vehicle_type = models.CharField(max_length=100, blank=True, choices=type_list, default="SUV")
    def __str__(self):
        return self.plate_number

class Owner(models.Model):
    user = models.OneToOneField(User, on_delete = models.CASCADE)
    is_owner = models.BooleanField(default=False)
    def __str__(self):
        return f'{self.user}'

class Sharer(models.Model):
    user = models.OneToOneField(User, on_delete = models.CASCADE)
    is_sharer= models.BooleanField(default=False)
    def __str__(self):
        return f'{self.user}'


class Ride(models.Model):
    driver = models.CharField(max_length=100, unique=True, blank=True, null=True)
    owner = models.CharField(max_length=100, unique=True, blank=True, null=True)
    sharer = models.CharField(max_length=100, unique=False, blank=True, null=True)
    destination = models.CharField(max_length=100)
    arrival_time = models.DateTimeField(default = django.utils.timezone.now())
    total_pass = models.IntegerField(default=1)
    sharing = models.BooleanField(default=False)
    status = models.CharField(max_length=100, default="Open")
    vehicle_type = models.CharField(max_length=100, blank=True, choices=type_list, default="SUV")
    other_request = models.CharField(max_length=200, blank=True)


    def __str__(self):
        return self.destination