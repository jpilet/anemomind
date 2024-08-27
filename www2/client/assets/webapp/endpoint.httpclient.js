import { makeEndpointConstructor } from './httputils.js';
import {schema} from './endpoint-schema.js';

export const tryMakeEndpoint = makeEndpointConstructor(schema);
