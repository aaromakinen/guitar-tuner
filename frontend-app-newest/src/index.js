import React from 'react';
import ReactDOM from 'react-dom';
import { Provider } from 'react-redux';
import { createStore, applyMiddleware } from 'redux';
import { BrowserRouter, Route, Switch } from 'react-router-dom';
import CssBaseline from '@material-ui/core/CssBaseline';
import promise from 'redux-promise';

import reducers from './reducers';
import TuningsIndex from './components/tunings_index';
import TuningsNew from './components/tunings_new';
import TuningsShow from './components/tunings_show';
import SignIn from './pages/sign-in.js';

const createStoreWithMiddleware = applyMiddleware(promise)(createStore);

ReactDOM.render(
  <Provider store={createStoreWithMiddleware(reducers)}>
  <React.Fragment>
      <CssBaseline />
      <BrowserRouter>
      <div>
      <Switch>
      <Route path="/tunings/new" component={TuningsNew} />
      <Route path="/tunings/:id" component={TuningsShow} />
      <Route path="/sign-in" component={SignIn} />
      <Route path="/" component={TuningsIndex} />
      </Switch>
      </div>
      </BrowserRouter>

    </React.Fragment>
  </Provider>
  , document.querySelector('.container'));
