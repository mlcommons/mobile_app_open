import * as functions from "firebase-functions";
import * as admin from "firebase-admin";
admin.initializeApp();
const db = admin.firestore();

import * as express from 'express';
const app = express()
export const v0 = functions.https.onRequest(app);

import {auth} from './auth';
import {upload} from './upload';

app.post("/upload", auth(admin.auth()), upload(db));
