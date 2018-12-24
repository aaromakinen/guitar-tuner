# guitar-tuner-app
A web application for an automatic Guitar Tuner Device.
This backend web application is written using Python Flask Framework, it provides necessary API Endpoint for the Frontend Application.

Some Endpoints:
/tunings : List of all current avalable tuning presets
/song/<string:name> : Get infomation of a Song
/tuning/<int:tuning_id>: Get the tuning Infomation by ID
/selected" : Current selected Tuning Preset


Besides, there are other endpoint that currently has no use (but it is already
  implemented)

/songs : List of all songs avalable
/register: Register new user
/user/<int:user_id>: Get User Infomation
/login: User Login
/refresh: Refresh Token
/logout: User Logout

# Installation Instruction

Install Pipenv (if it is not installed)
------

``` pip install pipenv```

From the root folder of the application, initilize a new virtual environment (which is managed by Pipenv):
------

```pipenv shell```

Install the packages automatically:
------

```pipenv install```

With the above command, all the packages that are listed in Pipfile will be
installed on the above virtual enviroment.

To start the application:
------

```python app.py```


To exit the virtual environment:
------

```exit```
