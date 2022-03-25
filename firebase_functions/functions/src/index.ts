import * as functions from "firebase-functions";
import * as admin from "firebase-admin";
admin.initializeApp();
const db = admin.firestore();
import * as cors from 'cors';

import * as express from 'express';
const app = express()
export const v0 = functions.https.onRequest(app);

import {auth} from './auth';
import {upload} from './upload';
import {fetchFirst, fetchNext, fetchPrev, fetchId} from './fetch';

app.use(cors({origin: true}));

app.post("/upload", auth(admin.auth()), upload(db));
app.get("/fetch/first", auth(admin.auth()), fetchFirst(db));
app.get("/fetch/next", auth(admin.auth()), fetchNext(db));
app.get("/fetch/prev", auth(admin.auth()), fetchPrev(db));
app.get("/fetch/id", auth(admin.auth()), fetchId(db));
