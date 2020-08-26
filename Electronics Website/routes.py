# Importing Libraries
from flask import render_template, Flask, request, flash, redirect, url_for
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
    times = db.relationship('UserTimes', back_populates='user')
    queue = db.relationship('ScannerQueue', back_populates='user')

class UserTimes(db.Model):
    __tablename__ = 'usertimes'
    
    id = db.Column(db.Integer, primary_key=True, unique=True, nullable=False)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'))
    time = db.Column(db.DateTime)
    user = db.relationship('User', back_populates='times')

class ScannerQueue(db.Model):
    __tablename__ = 'scannerqueue'
    
    id = db.Column(db.Integer, primary_key=True, unique=True, nullable=False)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'))
    scanner = db.Column(db.Integer, db.ForeignKey('scanner.id'))
    user = db.relationship('User', back_populates='queue')
    scanners = db.relationship('Scanner', back_populates='queue')

class Scanner(db.Model):
    __tablename__ = 'scanner'
    
    id = db.Column(db.Integer, primary_key=True, unique=True, nullable=False)
    name = db.Column(db.String(10), nullable=False, unique=True)
    queue = db.relationship('ScannerQueue', back_populates='scanners')

db.create_all()

# Route of home page
@app.route('/', methods=['GET'])
@app.route('/home', methods=['GET'])
def index():
    times = UserTimes.query.all()
    if times:
        times_list = [(i.user.name, i.time) for i in times]
        return render_template('home.html', times=times_list)
    else:
        return render_template('home.html', times=None)

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
        return 'Failed To Obtain Values'
    else:
        # Checks if the user id and uid exist
        if user_code and uid:
            user_code.strip("�?\n")
            user_code = int(user_code) if type(user_code) != int else user_code
            uid = uid.strip()
            time = datetime.datetime.now()
            user = User.query.filter_by(user_code=user_code).first()
            if user.uid != uid:
                return 'Invalid Card'
        
            try:
                new_time = UserTimes(user=user.id, time=time)
                db.session.add(new_time)
                db.session.flush()
            except:
                db.session.rollback()
                return 'Error Occured'
            else:
                db.session.commit()
                return 'Success'

@app.route('/add_scanner', methods=['GET', 'POST'])
def add_scanner():
    form = ScannerForm()
    if request.method == 'POST' and form.validate_on_submit():
        scanner_id = form.name.data.strip()
        if Scanner.query.filter_by(name=scanner_id).first():
            flash("Scanner Already Exists")
        else:
            try:
                new_scanner = Scanner(name=scanner_id)
                db.session.add(new_scanner)
                db.session.flush()
            except:
                flash('An Error Occured, Scanner was not Added')
                db.session.rollback()
            else:
                db.session.commit()
                flash('Scanner was successfully added')
    scanners = Scanner.query.all()
    return render_template('scanner.html', form=form, scanners=scanners)
    

@app.route('/queue_user', methods=['GET', 'POST'])
def queue_user():
    form = QueueForm()
    form.user.choices = [(x.id, x.name) for x in User.query.all()]
    form.scanner.choices = [(x.id, x.name) for x in Scanner.query.all()]
    if request.method == 'POST' and form.validate_on_submit():
        if ScannerQueue.query.filter_by(user_id=form.user.data, scanner=form.scanner.data).first():
            flash('User is already queued to scanner')
        else:
            try:
                new_queue = ScannerQueue(user_id = form.user.data, scanner=form.scanner.data)
                db.session.add(new_queue)
                db.session.flush()
            except:
                flash('An Error Occured')
                db.session.rollback()
            else:
                db.session.commit()
                flash('Successfully added to queue')
    queue = ScannerQueue.query.all()
    return render_template('queue.html', form=form, queue=queue)

@app.route('/get_data/<scanner>/<data_type>', methods=['GET'])
def getdata(scanner, data_type):
    user = ScannerQueue.query.filter_by(scanner=scanner.strip()).first()
    if user:
        if data_type == "name":
            return user.user.name
        elif data_type == "code":
            return user.user.user_code
    return 'No User'

@app.route('/receive_data', methods=['POST'])
def receivedata():
    try:
        # Gets user id and uid of card from the JSON data of the post request
        user_code = request.form['user_code']
        uid = request.form['card_id']
        scanner = request.form['scanner']
    except:
        return 'Failed To Obtain Values'
    else:
        # Checks if the user id and uid exist
        if user_code and uid:
            user_code = int(user_code) if type(user_code) != int else user_code
            uid = uid.strip()
            user = User.query.filter(user_code=user_code).first()
            try:
                user.uid = uid
                db.session.flush()
            except:
                db.session.rollback()
                return 'An Error Occured'
            else:
                db.session.commit()
                
                delete_scannerqueue = ScannerQueue.query.filter_by(scanner=scanner, user_id=user.id).first()
                db.session.delete(delete_scannerqueue)
                try:
                    db.session.flush()
                except:
                    db.session.rollback()
                else:
                    db.session.commit()
                return "User's card updated"


# Error handling route for 404, page not found
@app.errorhandler(404)
def error(e):
    flash('Could not find that page (Error 404)')
    return redirect(url_for('index'))

# Error handling route for 405, invalid request method
@app.errorhandler(405)
def error(e):
    flash('Invalid Request Method')
    return redirect(url_for('index'))


# Runs app if the script is run directly
if __name__ == '__main__':
    #app.run(host='0.0.0.0', debug=True, port=80)
    app.run(debug=True)