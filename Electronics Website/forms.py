from flask_wtf import FlaskForm
from wtforms import StringField, SubmitField, IntegerField
from wtforms.validators import InputRequired, length

class RegisterForm(FlaskForm):
    user = StringField('Name', validators=[InputRequired(message='Name Required'),
                                           length(1, 30)])
    user_code = IntegerField('User Code', validators=[InputRequired(message='User code required')])
    submit = SubmitField('Submit')