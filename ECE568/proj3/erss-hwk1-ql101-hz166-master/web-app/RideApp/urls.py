from django.urls import path
from .views import DriverOrderListView, DriverOrderDetailView, OrderDetailView, OrderListView, OrderCreateView, OrderUpdateView, OrderDeleteView, DriverUpdateForm, SharerOrderListView
from . import views


# urlpatterns = [
#     #path('', views.index, name='index'),#对应<a href="{% url 'index' %}">Home</a>.
#     path('', views.welcome, name='welcome'),
#     path('index/', views.index,name='index'),
# ]

app_name = 'RideApp'
urlpatterns = [
    path('', views.home, name='home'),
    path('home/', views.home, name='home'),
    path('driver/', views.driver_home, name='driver-home'),
    path('driver/search/', views.driver_search, name='driver-search'),
    path('driver/orders/<int:pk>/', DriverOrderListView.as_view(), name='driver-orders'),
    path('order/<int:pk>/detail/', DriverOrderDetailView.as_view(), name='driver-order-detail'),
    path('driver/order_confirm/<int:pk>/', views.driver_confirm, name='driver-order-confirm'),
    path('driver/order_complete/<int:pk>/', views.driver_complete, name='driver-order-complete'),
    path('driver/update/', views.driver_update, name='driver-update'),
    path('driver/info/<int:pk>', views.driver_info, name='driver-info'),


    path('home/userAllRequest/', OrderListView.as_view(), name='userAllRequest'),
    path('home/createRequest/', OrderCreateView.as_view(), name='order_form'),
    path('home/userAllRequest/order/<int:pk>/', OrderDetailView.as_view(), name='order-detail'),
    path('home/userAllRequest/order/<int:pk>/update/', OrderUpdateView.as_view(), name='order-update'),
    path('home/userAllRequest/order/<int:pk>/delete/', OrderDeleteView.as_view(), name='order-delete'),


    path('sharer/search/', views.sharer_search, name='sharer-search'),
    path('sharer/join/<int:order_pk>/<int:sharer_pk>/', views.sharer_join, name='sharer-join'),
    path('sharer/home/', views.sharer_home, name='sharer-home'),
    path('sharer/list/<int:pk>', SharerOrderListView.as_view(), name='sharer-list'),
]
