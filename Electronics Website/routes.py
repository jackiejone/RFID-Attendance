# Importing Libraries
from flask import render_template, Flask, request
from flask_sqlalchemy import SQLAlchemy
import os

# Defining path and file of database
project_dir = os.path.dirname(os.path.abspath(__file__))
database_file = "sqlite:///{}".format(os.path.join(project_dir, "Attendance.db"))

# Configuring Flask app
app = Flask(__name__)

# Defining database for app
app.config["SQLALCHEMY_DATABASE_URI"] = database_file
db = SQLAlchemy(app)

# Creating database model
class User(db.Model):
    __tablename__ = 'user'
    
    id = db.Column(db.Integer, primary_key=True, unique=True, nullable=False)
    user_code = db.Column(db.Integer, nullable=False)
    uid = db.Column(db.Text(30), nullable=False)

db.create_all()

# Route of home page
@app.route('/')
def index():
    return "<h1>RFID Attendance</h1>"

# Route to insert data into the database
@app.route('/insert', methods=['POST'])
def ins():
    # Try statement to avoid any errors when getting the data from the 
    try:
        # Gets user id and uid of card from the JSON data of the post request
        user_id = request.form['user']
        uid = request.form['uid']
    except:
        print('failed to obtain values')
    else:
        # Checks if the user id and uid exist
        if user_id and uid:
            # Checking if the user has already been added to the database
            if User.query.filter_by(user_code=user_id).first():
                print('duplicate')
            else:
                # Adds a new user to the database
                new_user = User(user_code=user_id, uid=uid)
                db.session.add(new_user)
                db.session.commit()
                print('Successfully added')
    return user_id, uid

# Gets the data of a specific user and returns the id, user_id, and uid of the user
@app.route('/get_data/<user>', methods=['GET'])
def getdata(user):
    print(user)
    user_object = User.query.filter_by(user_code=user).first()
    if user_object:
        return f"{user_object.id} {user_object.user_code} {user_object.uid}"
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