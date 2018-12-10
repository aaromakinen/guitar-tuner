import React, { Component } from 'react';
import { Field, reduxForm } from 'redux-form';
import { Link } from 'react-router-dom';
import { connect } from 'react-redux';
import { createTuning } from '../actions';

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
      <h3>Awesome, let's create your own professional tuning preset!</h3>
      <form  onSubmit={handleSubmit(this.onSubmit.bind(this))}>
        <Field
          label="Tuning Name"
          name="name"
          component={this.renderField}
        />
        <Field
          label="String One"
          name="str_one"
          component={this.renderField}
        />
        <Field
          label="String Two"
          name="str_two"
          component={this.renderField}
        />
        <Field
          label="String Three"
          name="str_three"
          component={this.renderField}
        />
        <Field
          label="String Four"
          name="str_four"
          component={this.renderField}
        />
        <Field
          label="String Five"
          name="str_five"
          component={this.renderField}
        />
        <Field
          label="String Six"
          name="str_six"
          component={this.renderField}
        />
        <button type="submit" className="btn btn-primary"> Save </button>
        <Link className="btn btn-danger" to="/"> Cancel </Link>
      </form>
      </div>
    );
  };
}
function validate(values) {
  const errors = {};
  let str_one = parseFloat(values.str_one)
  let str_two = parseFloat(values.str_two)
  let str_three = parseFloat(values.str_three)
  let str_four = parseFloat(values.str_four)
  let str_five = parseFloat(values.str_five)
  let str_six = parseFloat(values.str_six)

  // Validate the inputs from 'values'
  // these below block is bad code -> remember to change it.
  if (!values.name) {
    errors.name = "Enter a title";
  } else {
    if (values.name.length > 18) {
      errors.name = "Name has to be maximum of 18 characters."
    }
  }

  if (!values.str_one || isNaN(values.str_one)) {
    errors.str_one = "Enter frequency, please!";
  }
  if (!values.str_two || isNaN(values.str_two)) {
    errors.str_two = "Enter frequency, please!";
  }
  if (!values.str_three || isNaN(values.str_three)) {
    errors.str_three = "Enter frequency, please!";
  }
  if (!values.str_four || isNaN(values.str_four)) {
    errors.str_four = "Enter frequency, please!";
  }
  if (!values.str_five || isNaN(values.str_five)) {
    errors.str_five = "Enter frequency, please!";
  }
  if (!values.str_six || isNaN(values.str_six)) {
    errors.str_six = "Enter frequency, please!";
  }

  // Check if the frequency is out of range.
  if (str_one >= 112.00 || str_one <= 70.00) {
    errors.str_one = "Frequency out of range, should be between 112 and 70";
  }

  if (str_two >= 140.00 || str_two <= 90.00) {
    errors.str_two = "Frequency out of range, should be between 140 and 90";
  }

  if (str_three >= 176.00 || str_three <= 116.00) {
    errors.str_three = "Frequency out of range, should be between 176 and 116";
  }
  if (str_four >= 226.00 || str_four <= 166.00) {
    errors.str_four = "Frequency out of range, should be between 226 and 166";
  }

  if (str_five >= 276.00 || str_five <= 216.00) {
    errors.str_five = "Frequency out of range, should be between 276 and 216";
  }

  if (str_six >= 360.00 || str_six <= 300.00) {
    errors.str_six = "Frequency out of range, should be between 360 and 300";
  }

  return errors;
}

export default reduxForm({
  validate,
  form: 'TuningsNewForm'
})(
  connect(null,{ createTuning })(TuningsNew)
);
