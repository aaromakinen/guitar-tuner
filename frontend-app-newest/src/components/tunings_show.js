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
        <table className="table">
          <tr>
            <th>String</th>
            <th>Frequency</th>
          </tr>
          <tr>
            <td>One</td>
            <td>{tuning.str_one} Hz</td>
          </tr>
          <tr>
            <td>Two</td>
            <td>{tuning.str_two} Hz</td>
          </tr>
          <tr>
            <td>Three</td>
            <td>{tuning.str_three} Hz</td>
          </tr>
          <tr>
            <td>Four</td>
            <td>{tuning.str_four} Hz</td>
          </tr>
          <tr>
            <td>Five</td>
            <td>{tuning.str_five} Hz</td>
          </tr>
          <tr>
            <td>Six</td>
            <td>{tuning.str_six} Hz</td>
          </tr>

        </table>

        </div>
        <button
        className="btn btn-primary pull-xs-right"
        onClick={this.onSelectClick.bind(this)}
        >
        Select This Tuning
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
