import React, { Component } from 'react';
import { connect } from 'react-redux';
import { Link } from 'react-router-dom';
import { fetchTuning, deleteTuning, selectTuning } from '../actions';


class TuningsShow extends Component {
  componentDidMount() {
      const { id } = this.props.match.params;
      this.props.fetchTuning(id);
  }

  onDeleteClick() {
    const { id } = this.props.match.params;
    this.props.deleteTuning(id, () => {
      this.props.history.push("/");
    });
  }

  onSelectClick() {
    const { id } = this.props.match.params;
    this.props.selectTuning(id, () => {
    this.props.history.push('/');
    });
  }

  render() {
    const { tuning } = this.props;
    if (!tuning) {
      return <div> Loading...</div>
    } else {
      return (
        <div className="container-fluid">
          <div className="menu-bar">
            <Link to="/">Back to List</Link>
            <button
              className="btn btn-danger pull-xs-right right-corner"
              onClick={this.onDeleteClick.bind(this)}
            >
              Delete
            </button>
          </div>
          <h5>Detail of {tuning.name}</h5>

        <div className="list-string">
        <ul>
          <li>1st -------------: {tuning.str_one} Hz</li>
          <li>2nd -------------: {tuning.str_two} Hz</li>
          <li>3rd -------------: {tuning.str_three} Hz</li>
          <li>4th -------------: {tuning.str_four} Hz</li>
          <li>5th -------------: {tuning.str_five} Hz</li>
          <li>6th -------------: {tuning.str_six} Hz</li>
        </ul>
        </div>
        <button
        className="btn pull-xs-right"
        onClick={this.onSelectClick.bind(this)}
        >
        Select this Tuning for my Guitar
        </button>
        </div>
      );
    }
  }
}

function mapStateToProps({ tunings }, ownProps) {
  return { tuning: tunings[ownProps.match.params.id] };
}

export default connect(mapStateToProps, { fetchTuning, deleteTuning, selectTuning })(TuningsShow);
