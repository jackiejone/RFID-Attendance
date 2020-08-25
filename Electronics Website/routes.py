# Importing Libraries
from flask import render_template, Flask, request, flash
from flask_sqlalchemy import SQLAlchemy
from forms import *
import datetime
import os

# Defining path and file of database
project_dir = os.path.dirname(os.path.abspath(__file__))
database_file = "sqlite:///{}".format(os.path.join(project_dir, "Attendance.db"))

# Configuring Flask app
app = Flask(__name__)

# Defining database for app
app.config['SECRET_KEY'] = os.urandom(16)
app.config["SQLALCHEMY_DATABASE_URI"] = database_file
db = SQLAlchemy(app)


# Creating database model
class User(db.Model):
    __tablename__ = 'user'
    
    id = db.Column(db.Integer, primary_key=True, unique=True, nullable=False)
    name = db.Column(db.String(20), nullable=False)
    user_code = db.Column(db.Integer, nullable=False)
    uid = db.Column(db.Text(30), nullable=True)
    times = db.relationship('UserTimes', back_populates='user_time')
    queue = db.relationship('ScannerQeue', back_populates='user')

class UserTimes(db.Model):
    __tablename__ = 'usertimes'
    
    id = db.Column(db.Integer, primary_key=True, unique=True, nullable=False)
    user = db.Column(db.Integer, db.ForeignKey('user.id'))
    time = db.Column(db.DateTime)
    user = db.relationship('User', back_populates='times')

class ScannerQueue(db.Model):
    __tablename__ = 'scannerqueue'
    id = db.Column(db.Integer, primary_key=True, unique=True, nullable=False)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'))
    scanner = db.Column(db.String(10))
    user = db.relationship('User', back_populates='queue')

db.create_all()

# Route of home page
@app.route('/', methods=['GET'])
@app.route('/home', methods=['GET'])
def index():
    times = UserTimes.query.all()
    times_list = [(i.user.name, i.time) for i in times]
    return render_template('home.html', times=times_list)

@app.route('/register', methods=['GET', 'POST'])
def register():
    form = RegisterForm()
    if request.method == 'POST' and form.validate_on_submit():
        name = form.user.data.strip().lower()
        user_code = form.user_code.data
        if User.query.filter_by(user_code=user_code).first():
            print(User.query.filter_by(name=name, user_code=user_code).first())
            flash('Student Code Already Exists')
            return render_template('register.html', form=form)
        try:
            new_user = User(name=name, user_code=user_code)
            db.session.add(new_user)
            db.session.flush()
        except ValueError:
            flash('An Error occued')
            db.session.rollback()
        else:
            db.session.commit()
            flash('Successfully Added Student')
    return render_template('register.html', form=form)

# Route to insert data into the database
@app.route('/insert', methods=['POST'])
def ins():
    # Try statement to avoid any errors when getting the data from the 
    try:
        # Gets user id and uid of card from the JSON data of the post request
        user_code = request.form['user_code']
        uid = request.form['card_id']
    except:
        print('failed to obtain values')
    else:
        # Checks if the user id and uid exist
        if user_code and uid:
            user_code = int(user_code) if type(user_code) != int else user_code
            uid = uid.strip()
            time = datetime.datetime.now()
            user = User.query.filter(user_code=user_code).first()
            if user.uid != uid:
                return('Invalid Card For User')
        
            try:
                new_time = UserTimes(user=user.id, time=time)
                db.session.add(new_time)
                db.session.flush()
            except:
                db.session.rollback()
                return('An Error Occured')
            else:
                db.session.commit()
                return 'Success'

# Gets the data of a specific user and returns the id, user_id, and uid of the user
@app.route('/get_data/<scanner>', methods=['GET'])
def getdata(scanner):
    user = ScannerQueue.query.filter_by(scanner=scanner.strip()).first()
    if user:
        return
    return 'No User'


# Error handling route for 404, page not found
@app.errorhandler(404)
def error(e):
    return "Error, page not found"

# Error handling route for 405, invalid request method
@app.errorhandler(405)
def error(e):
    return "Error, Invalid method for route"


# Runs app if the script is run directly
if __name__ == '__main__':
    app.run(host='0.0.0.0', debug=True, port=80)