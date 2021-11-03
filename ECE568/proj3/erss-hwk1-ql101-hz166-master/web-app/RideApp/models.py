from django.db import models
from django.utils import timezone
from django.contrib.auth.models import User
from django.urls import reverse


class RideUser(models.Model):
    person = models.OneToOneField(User, on_delete=models.CASCADE)
    is_driver = models.BooleanField(default=False)
    vehicle_type = models.CharField(max_length=50,null=True,blank=True,help_text="user's vehicle type")
    special_request = models.CharField(max_length=50,null=True,blank=True,help_text="user's special reequest")
    max_car_pass = models.IntegerField(null=True,blank=True,help_text="car's max passenger")
    license_plate_number = models.CharField(max_length=50,null=True,blank=True,help_text="user's license_plate_number")

    def __str__(self):
        return self.person.username


class Order(models.Model):
    owner = models.ForeignKey(User, on_delete=models.CASCADE,null=True,related_name='owner')#owener
    driver = models.ForeignKey(User, on_delete=models.CASCADE,null=True,related_name='driver')#driver
    dst = models.CharField(max_length=200, verbose_name='Destination')
    arrivial_time = models.DateTimeField(help_text='Format: 2020-01-01 12:00', null=True)
    order_pass_num = models.IntegerField(null=True,default=0)
    is_share = models.BooleanField(default=False)
    owner_pass_num = models.IntegerField(null=True,default=0)
    special_request = models.TextField(max_length=200, help_text="Do you have any other requests",null=True, blank=True,
                                       default='null')  # match info
    special_vehicle_type = models.CharField(max_length=200, help_text="Car type", null=True, blank=True,
                                            default='null')  # match info



    STATUS = (
        ('open', 'open'),
        ('confirm', 'confirm'),
        ('complete', 'complete'),
    )

    status = models.CharField(max_length=20, choices=STATUS, default='open')

    def __str__(self):
        return self.dst

    def get_absolute_url(self):
        return reverse('RideApp:order-detail', kwargs={'pk': self.pk})  # 一定要加rideapp:,不然找不到


class Sharer(models.Model):
    sharer = models.ForeignKey(User, on_delete=models.CASCADE)
    request = models.ForeignKey(Order, on_delete=models.CASCADE, null=True)
    share_pass_num = models.IntegerField(null=True, default=0)
    early_time = models.DateTimeField(verbose_name='Early acceptable arrive date', help_text='Format: 2020-01-01 12:00')
    last_time = models.DateTimeField(verbose_name='Latest acceptable arrive date', help_text='Format: 2020-01-01 12:00')

    def __str__(self):
        return self.sharer.username

