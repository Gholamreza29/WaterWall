# Reality Tunnel

<div dir="rtl">

ریلیتی مدت هاست که استفاده میشه و همه میشناسن

این نسخه از ریلیتی که داخل واتروال استفاده میشه با روش های دیگه ای که برنامه نویس های دیگه نوشتن فرق میکنه اما درکل به یک هدف میرسند

ریلیتی به شما این اجازه رو میده که هندشیک رو با دامنه دلخواه تکمیل کنید و اگر کاکشن احراز نشد ؛ خیلی ساده فالبک بشه به همون سایت destination

درحقیقت سرور شما انگار iptalbe شده به destination و به خاطر همین تشخیصش توسط فایروال خیلی سخته

البته شاید راه های دیگه پیدا کرده باشن که خیلی درگیرش نشیم چون نمیخوام به کسی ایده بدم درکل ولی

خلاصه کلام اینکه ریلیتی رمزنگاری هم داره ؛ برای تونل رمزنگاری کامل و سنگین مثل aes نیاز نیست ولی خوب الان واتروال برای ریلیتی فقط همین الگوریتم رو داره

که خوب خیلی هم performance شاید نگیره چون hardware accelerated پیاده شده توسط openssl

ریلیتی در واتروال ساده پیاده شده ولی در عین حال تا جایی که من علمشو داشتم نکاتش رعایت شده 

شما تنها باید یک پسورد مشترک داشته باشید و همچنین یک sni که خودتون هرچی دوست داشتید میزنید


با اینکه یک پسورد مشترک استفاده میشه ؛ اما دیتای ارسالی کاملا tls simulated هست و همچین تمام پکت ها با کمک IV رندوم هستند و دیتای هیچ سمپلی تکرار نمیشه 

و همچنین امضا با hmac هم انجام میشه تا یوقت هکر ها اطلاعات با ارزش شما رو هک نکن و دامنه ای که کاربر میخواد وصل بشه رو پیدا نکنن !

امیدوارم این ها کافی بوده باشه و جایی سوتی نداده باشم 😁

ولی این جور چیز ها تا وقتی چند نفر متخصص توی امنیت برسی نکنن نمیشه نظر داد ؛ اگه کسی رو میشناسید یا خودتون فعال هستید تو این حیطه خوشحالم میشم 
کد رو برسی کنید و سوتی های فراوانی که دادم رو بهم اطلاع بدید

این نکات امنیتی الگوریتم aes بود ؛ بعدا براش الگوریتم مورد علاقه خودم که xor هست رو اضافه میکنم که هیچ کودم از این جزییات امنیتی روش رعایت نمیشه ولی پرفورمنس بهتری خواهد داد



 برای تونل مستقیم ریلیتی از این مثال میشه استفاده کرد


- ایپی سرور خارج 1.1.1.1
- رمز passwd
- sni: i.stack.imgur.com
- multi port (443 - 65535)

---
# سرور ایران

</div>


```json
{
    "name": "reality_client_multiport",
    "nodes": [
        {
            "name": "users_inbound",
            "type": "TcpListener",
            "settings": {
                "address": "0.0.0.0",
                "port": [443,65535],
                "nodelay": true
            },
            "next": "header"
        },
        {
            "name": "header",
            "type": "HeaderClient",
            "settings": {
                "data": "src_context->port"
            },
            "next": "my_reality_client"
        },
        {
            "name": "my_reality_client",
            "type": "RealityClient",
            "settings": {
                "sni":"i.stack.imgur.com",
                "password":"passwd"

            },
            "next": "outbound_to_kharej"
        },

        {
            "name": "outbound_to_kharej",
            "type": "TcpConnector",
            "settings": {
                "nodelay": true,
                "address":"1.1.1.1",
                "port":443
            }
        }
     
      
    ]
}
```

<div dir="rtl">

---
# سرور خارج




</div>

```json
{
    "name": "reality_server_multiport",
    "nodes": [
        {
            "name": "main_inbound",
            "type": "TcpListener",
            "settings": {
                "address": "0.0.0.0",
                "port": 443,
                "nodelay": true
            },
            "next": "my_reality_server"
        },

        {
            "name": "my_reality_server",
            "type": "RealityServer",
            "settings": {
                "destination":"reality_dest_node",
                "password":"passwd"

            },
            "next": "header_server"
        },
        
        {
            "name": "header_server",
            "type": "HeaderServer",
            "settings": {
                "override": "dest_context->port"
            },
            "next": "final_outbound"
        },

        {
            "name": "final_outbound",
            "type": "TcpConnector",
            "settings": {
                "nodelay": true,
                "address":"127.0.0.1",
                "port":"dest_context->port"

            }
        },

        {
            "name": "reality_dest_node",
            "type": "TcpConnector",
            "settings": {
                "nodelay": true,
                "address":"i.stack.imgur.com",
                "port":443
            }
        }
      
    ]
}


```

<div dir="rtl">






