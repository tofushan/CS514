from django.contrib.auth.models import User
from django.views.generic import ListView, DetailView

from .models import Order, RideUser, Sharer
from django.views import generic
from django.contrib.auth.mixins import LoginRequiredMixin, UserPassesTestMixin
from django.contrib.auth.decorators import login_required
from django.shortcuts import get_object_or_404, render, redirect
from django.urls import reverse, reverse_lazy
from django.core.mail import send_mail
from django.utils import timezone as datetime
from datetime import datetime
from django.contrib import messages
import datetime
from django.contrib.auth.forms import UserCreationForm
from .forms import UserRegisterForm, DriverUpdateForm, SharerSearchForm


def register(request):
    if request.method == 'POST':
        form = UserRegisterForm(request.POST)
        if form.is_valid():
            form.save()
            username = form.cleaned_data.get('username')
            messages.success(request, f'Account created for {username}!')
            return redirect('RideApp:home')
    else:
        form = UserRegisterForm()
    return render(request, 'RideApp/register.html', {'form': form})


def home(request):
    return render(request, 'RideApp/home.html')


class OrderListView(generic.ListView):
    model = Order
    template_name = 'RideApp/userAllRequest.html'
    context_object_name = 'orders'



class OrderDetailView(generic.DetailView):
    model = Order


class OrderCreateView(LoginRequiredMixin, generic.CreateView):
    model = Order
    # name = model.owner.username

    fields = ['dst', 'arrivial_time', 'is_share', 'owner_pass_num', 'special_request', 'special_vehicle_type']

    def form_valid(self, form):
        form.instance.owner = self.request.user
        form.instance.order_pass_num = self.request.POST.get('owner_pass_num')
        return super().form_valid(form)  # 将当前用户变成owner


class OrderUpdateView(LoginRequiredMixin, UserPassesTestMixin, generic.UpdateView):
    model = Order
    # name = model.owner.username

    fields = ['dst', 'arrivial_time', 'is_share', 'owner_pass_num', 'special_request', 'special_vehicle_type']

    def form_valid(self, form):
        form.instance.owner = self.request.user
        form.instance.order_pass_num = self.request.POST.get('owner_pass_num')
        return super().form_valid(form)  # 将当前用户变成owner


    def test_func(self):  # in case other user change the file
        order = self.get_object()
        if order.owner == self.request.user:
            if order.status == 'open':
                return True
        return False


class OrderDeleteView(LoginRequiredMixin, UserPassesTestMixin, generic.DeleteView):
    model = Order
    success_url = reverse_lazy('RideApp:userAllRequest')  # 删除成功后，页面回到哪里

    def test_func(self):
        order = self.get_object()
        if self.request.user == order.owner:
            if order.status != 'confirm':
                return True
        return False



# divide
# divide
# divide
# divide



#  views for driver
@login_required
def driver_home(request):
    user = request.user
    if RideUser.objects.filter(person=user).exists():
        ride_user = get_object_or_404(RideUser, person=user)
        if ride_user.is_driver:
            return render(request, 'RideApp/driver_home.html')
        else:
            return redirect('RideApp:driver-update')
    else:
        return redirect('RideApp:driver-update')



class DriverOrderListView(ListView):
    model = Order
    context_object_name = 'orders'
    template_name = 'RideApp/driver_lists.html'

    def get_queryset(self):
        user = get_object_or_404(User, pk=self.kwargs.get('pk'))
        return Order.objects.filter(driver=user, status='confirm')


class DriverOrderDetailView(DetailView):
    model = Order
    template_name = 'RideApp/driver_list_detail.html'

    def get_context_data(self, **kwargs):
        context = super().get_context_data(**kwargs)
        order = get_object_or_404(Order, pk=self.kwargs.get('pk'))
        sharer_list = order.sharer_set.all()
        # Add in a QuerySet of all the books
        context['sharer_list'] = sharer_list
        return context


@login_required
def driver_search(request):
        rideuser = request.user.rideuser
        open_order = Order.objects.filter(status="open")
        selected_orders = open_order.filter(order_pass_num__range=(0, rideuser.max_car_pass), special_request=rideuser.special_request, special_vehicle_type=rideuser.vehicle_type)
        # 不能接自己的单 owner 或 sharer 是自己
        selected_orders = selected_orders.exclude(owner=request.user)

        for order in selected_orders:
            sharer_list = order.sharer_set.all()
            for sharer in sharer_list:
                if sharer is request.user:
                    selected_orders = selected_orders.exclude(order)

        return render(request, 'RideApp/selected_order_list.html', {'selected_orders': selected_orders})


@login_required
def driver_confirm(request, pk):
    order = get_object_or_404(Order, pk=pk)
    user = request.user
    order.status = 'confirm'
    order.driver = user
    order.save()
    if order.status == 'confirm':
        messages.success(request, f'Your have confirm the order!')
        subject = 'Order has been confirmed!'
        message = 'please wait for driver picking up.'
        from_email = 'luq1016@outlook.com'
        sharerSet = Sharer.objects.filter(request=order)
        to_list = [order.owner.email]
        send_mail(subject, message, from_email, to_list, fail_silently=False)
        for ssharer in sharerSet:
            send_mail(subject, message, from_email, [ssharer.sharer.email], fail_silently=False)
    return redirect('RideApp:driver-home')


@login_required
def driver_complete(request, pk):
    order = get_object_or_404(Order, pk=pk)
    order.status = 'complete'
    order.save()
    if order.status == 'complete':
        messages.success(request, f'Your have complete the order!')
    return redirect('RideApp:driver-home')


def driver_update(request):
    try:
        rideuser = request.user.rideuser
    except RideUser.DoesNotExist:
        rideuser = RideUser(person=request.user)

    if request.method == 'POST':
        form = DriverUpdateForm(request.POST, instance=rideuser)
        if form.is_valid():
            form.save()
            messages.success(request, f'Your has  become a driver!')
            return redirect('RideApp:driver-home')
    else:
        form = DriverUpdateForm(instance=rideuser)
        context = {
            'form': form,
        }
    return render(request, 'RideApp/driver_update.html', context)


def driver_info(request, pk):
    order = get_object_or_404(Order, pk=pk)
    driver = order.driver.rideuser
    vehicle_type = driver.vehicle_type
    max_car_pass = driver.max_car_pass
    license_plate_number = driver.license_plate_number
    special_request = driver.special_request
    context = { 'vehicle_type': vehicle_type,
                'max_car_pass': max_car_pass,
                'license_plate_number': license_plate_number,
                'special_request': special_request,
    }
    return render(request, 'RideApp/driver_info.html', context)


#  divide
#  divide
#  divide



#  sharer 不能找到自己的 订单 && 自己加过的订单

@login_required
def sharer_search(request):
    if request.method == 'POST':
        form = SharerSearchForm(request.POST)
        if form.is_valid():
            dst = form.cleaned_data.get('dst')
            early_time = form.cleaned_data.get('early_time')
            last_time = form.cleaned_data.get('last_time')
            share_pass_num = form.cleaned_data.get('share_pass_num')
            sharer = Sharer(sharer=request.user, share_pass_num=share_pass_num, early_time=early_time, last_time=last_time)
            sharer.save()
            open_order = Order.objects.filter(status="open")
            selected_orders = open_order.filter(arrivial_time__range=(early_time, last_time),
                                                dst=dst, is_share=True, status='open')
            selected_orders = selected_orders.exclude(owner=request.user)
            for order in selected_orders:
                sharer_list = order.sharer_set.all()
                if sharer_list.filter(pk=sharer.pk).exists():
                    selected_orders.exclude(pk=order.pk)
                if order.owner == request.user:
                    selected_orders.exclude(pk=order.pk)

            context = {'selected_orders': selected_orders, 'sharer_pk': sharer.pk}
            return render(request, 'RideApp/sharer_orders.html', context)
    else:
        form = SharerSearchForm()
    return render(request, 'RideApp/sharer_search.html', {'form': form})


#  通过 user && order 找到对应 的sharer
@login_required
def sharer_join(request, order_pk, sharer_pk):
    order = get_object_or_404(Order, pk=order_pk)
    sharer = Sharer.objects.get(pk=sharer_pk)
    sharer.request = order
    sharer.save()
    order.order_pass_num += sharer.share_pass_num
    order.save()
    if sharer.request == order:
        messages.success(request, f'Your have join the order!')
    return redirect('RideApp:home')


@login_required
def sharer_home(request):

    return render(request, 'RideApp/sharer_home.html')


class SharerOrderListView(ListView):
    model = Order
    context_object_name = 'orders'
    template_name = 'RideApp/sharer_lists.html'

    def get_queryset(self):
        user = get_object_or_404(User, pk=self.kwargs.get('pk'))
        sharer_list = user.sharer_set.all()
        order_list = Order.objects.all()
        for sharer in sharer_list:
            if sharer.request:
                order_list = order_list.exclude(pk=sharer.request.pk)
        ret = Order.objects.all()
        for order in order_list:
            ret = ret.exclude(pk=order.pk)
        return ret
