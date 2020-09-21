from flask_wtf import FlaskForm
from wtforms import StringField, SubmitField, IntegerField, SelectField
from wtforms.validators import InputRequired, length, ValidationError

def my_int_check(form, field):
    try:
        int(field.data)
    except ValueError:
        raise ValidationError('User code was not an integer')


class RegisterForm(FlaskForm):
    user = StringField('Name', validators=[InputRequired(message='Name Required'),
                                           length(1, 30)])
    user_code = StringField('User Code', validators=[InputRequired(message='User code required'), length(min=4, max=5), my_int_check])
    submit = SubmitField('Submit')

class QueueForm(FlaskForm):
    user = SelectField('User', coerce=int)
    scanner = SelectField('Scanner', coerce=int)
    queue = SubmitField('Queue')
    
class ScannerForm(FlaskForm):
    name = StringField('Scanner ID', validators=[InputRequired(message='Scanner ID Required'), length(1, 10)])
    add = SubmitField('Add')