import _ from 'lodash';
import React, { Component } from 'react';
import { connect } from 'react-redux';
import { Link } from 'react-router-dom';
import { fetchTunings } from '../actions';

class TuningsIndex extends Component {
  componentDidMount() {
      this.props.fetchTunings();
  }

  renderTunings() {
    return _.map(this.props.tunings, tuning => {
      if (tuning.selected) {
        return (
          <li className="list-group-item" key={tuning.id}>
          <Link to={`/tunings/${tuning.id}`}><span className="selected">{tuning.name}</span></Link>
          </li>
        )
      }
      return (

        <li className="list-group-item" key={tuning.id}>
          <Link to={`/tunings/${tuning.id}`}>{tuning.name}</Link>
        </li>
      );
    });
  }

  render() {
    return (
      <div className="container-fluid">
        <div className="text-xs-right">
          <Link className="btn btn-primary right-corner" to="/tunings/new">
            Add a Tuning
          </Link>
        </div>
        <p> Please select your tuning: </p>
        <ul className="list-group">
          {this.renderTunings()}
        </ul>
      </div>
    );
  }
}

function mapStateToProps(state) {
  return { tunings: state.tunings };
}

export default connect(mapStateToProps, { fetchTunings })(TuningsIndex);
