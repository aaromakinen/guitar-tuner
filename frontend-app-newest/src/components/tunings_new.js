import React, { Component } from 'react';
import { Field, reduxForm } from 'redux-form';
import { Link } from 'react-router-dom';
import { connect } from 'react-redux';
import { createTuning } from '../actions';
import Button from '@material-ui/core/Button';

class TuningsNew extends Component {
  renderField(field) {
    const { meta: {touched, error } } = field;
    const className = `form-group ${touched && error ? 'has-danger' : ''}`

    return (
      <div className={className}>
        <label> {field.label} </label>
          <input
            className="form-control"
            type="text"
            {...field.input}
          />
        <div className="text-help"> {touched ? error : ''}</div>
      </div>
    );
  }

  onSubmit(values) {
    this.props.createTuning(values, () => {
      this.props.history.push('/');
    });
  }

  render() {
    const { handleSubmit } = this.props;

    return (
      <div className="container-fluid">
        <div className="list-string">
          <p>Awesome, let's create your own professional tuning preset!</p>
          </div>
      <form className="list-group" onSubmit={handleSubmit(this.onSubmit.bind(this))}>
        <Field
          label="Tuning Name *"
          name="name"
          component={this.renderField}
        />
        <Field
          label="String One *"
          name="str_one"
          component={this.renderField}
        />
        <Field
          label="String Two *"
          name="str_two"
          component={this.renderField}
        />
        <Field
          label="String Three *"
          name="str_three"
          component={this.renderField}
        />
        <Field
          label="String Four *"
          name="str_four"
          component={this.renderField}
        />
        <Field
          label="String Five *"
          name="str_five"
          component={this.renderField}
        />
        <Field
          label="String Six *"
          name="str_six"
          component={this.renderField}
        />

        <button type="submit" className="btn btn-primary btn-save"> Create </button>
        <Link className="btn btn-danger" to="/"> Cancel </Link>
      </form>
      </div>
    );
  };
}

// Validate the Information provided by users, to prevent unwanted data.
function validate(values) {
  const errors = {};
  const requiredFields = [
    'str_one',
    'str_two',
    'str_three',
    'str_four',
    'str_five',
    'str_six'
  ];

  requiredFields.forEach(field => {
    if (!values[field] || isNaN(values[field])) {
      errors[field] = 'Enter Frequency in Hz';
    }
  })

  if (!values['name']) {
    errors['name'] = "Enter a title";
  } else {
    if (values['name'].length > 18) {
      errors['name'] = "Name has to be maximum of 18 characters.";
    }
  }

  let str_one = parseFloat(values.str_one);
  let str_two = parseFloat(values.str_two);
  let str_three = parseFloat(values.str_three);
  let str_four = parseFloat(values.str_four);
  let str_five = parseFloat(values.str_five);
  let str_six = parseFloat(values.str_six);

  // Check if the frequency is out of range.
  if (str_one >= 360.00  || str_one <= 290.00) {
    errors.str_one = "Frequency out of range, should be between 290 and 360";
  }

  if (str_two >= 276.0 || str_two <= 166.00) {
    errors.str_two = "Frequency out of range, should be between 166 and 276";
  }

  if (str_three >= 226.00 || str_three <= 166.00) {
    errors.str_three = "Frequency out of range, should be between 166 and 226";
  }
  if (str_four >= 176.00 || str_four <= 116.00) {
    errors.str_four = "Frequency out of range, should be between 116 and 176";
  }

  if (str_five >= 140.000 || str_five <= 90.00) {
    errors.str_five = "Frequency out of range, should be between 90 and 140";
  }

  if (str_six >= 112.00  || str_six <= 70.00) {
    errors.str_six = "Frequency out of range, should be between 70 and 112";
  }

  return errors;
}

export default reduxForm({
  validate,
  form: 'TuningsNewForm'
})(
  connect(null,{ createTuning })(TuningsNew)
);
