import * as functions from 'firebase-functions';
import * as admin from 'firebase-admin';
admin.initializeApp();
const db = admin.firestore();
import * as cors from 'cors';

import { currentPrefix } from './prefix.gen';

import * as express from 'express';
const app = express();
exports[currentPrefix] = functions.https.onRequest(app);

import { auth } from './auth';
import { upload } from './upload';
import { fetchNext, fetchPrev, fetchId } from './fetch';

app.use(cors({ origin: true }));

app.post('/upload', auth(admin.auth()), upload(db));
app.get('/fetch/next', fetchNext(db));
app.get('/fetch/prev', fetchPrev(db));
app.get('/fetch/id', fetchId(db));
