from flask import render_template, Flask, request
from flask_sqlalchemy import SQLAlchemy
import os

project_dir = os.path.dirname(os.path.abspath(__file__))
database_file = "sqlite:///{}".format(os.path.join(project_dir, "Attendance.db"))

app = Flask(__name__)
app.config["SQLALCHEMY_DATABASE_URI"] = database_file
db = SQLAlchemy(app)

class User(db.Model):
    __tablename__ = 'user'
    
    id = db.Column(db.Integer, primary_key=True, unique=True, nullable=False)
    user_code = db.Column(db.Integer, nullable=False)
    uid = db.Column(db.Text(30), nullable=False)

db.create_all()

@app.route('/')
def index():
    return "test"

@app.route('/insert', methods=['POST'])
def ins():
    try:        
        user_id = request.form['user']
        uid = request.form['uid']
    except:
        print('failed to obtain values')
    else:
        if user_id and uid:
            if User.query.filter_by(user_code=user_id).first():
                print('duplicate')
            else:
                new_user = User(user_code=user_id, uid=uid)
                db.session.add(new_user)
                db.session.commit()
                print('Successfully added')
    return user_id, uid

@app.route('/get_data/<user>', methods=['GET'])
def getdata(user):
    print(user)
    user_object = User.query.filter_by(user_code=user).first()
    if user_object:
        return f"{user_object.id} {user_object.user_code} {user_object.uid}"
    return 'No User'
    
@app.errorhandler(404)
def error(e):
    return "Error, page not found"


if __name__ == '__main__':
    app.run(host='0.0.0.0', debug=True, port=80)