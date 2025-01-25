# Authentication flow

From https://dev.twitch.tv/docs/authentication/

We need two authentication flows:
* (OAuth Authorization Code Grant Flow)[https://dev.twitch.tv/docs/authentication/getting-tokens-oauth/#authorization-code-grant-flow]
* (OAuth Client Credentials Grant Flow)[https://dev.twitch.tv/docs/authentication/getting-tokens-oauth/#client-credentials-grant-flow]

The first will provide us with a client authorization token.  The second will provide us with a application authorization token.

To get these we need:

* Client ID
* Client Secret
* User ID (only for the user auth token)

Headers:

(Note these are from the mock API)

-H 'Client-Id: 724fcdb8dd19248d3e386e0f282950'
-H 'Authorization: Bearer 6ea9bc97cd358d3'
```
curl -X POST 'http://localhost:8080/auth/authorize?client_id='^$CID^'&client_secret='^$SID^'&grant_type=user_token&user_id='^$TUID^'&scope='^<={%flatten %20 $scopes}

curl -X POST 'http://localhost:8080/auth/token?client_id='^$CID^'&client_secret='^$SID^'&grant_type=client_credentials'

scopes = channel:manage:broadcast

curl -H 'Accept: application/json' $HEADERS http://localhost:8080/mock/channels?broadcaster_id=^$TUID

; curl -v -X PATCH -H 'Content-Type: application/json' $HEADERS http://localhost:8080/mock/channels?broadcaster_id=^$TUID --data-raw '{"title": "This is a test change of the title 2222", "tags": ["English", "Coding", "Lalalala"], "content_classification_labels": []}'
```

```
Some Twitch API silliness: https://dev.twitch.tv/docs/authentication/scopes/ on that page run [...$('code')].slice(2).map(n => n.innerHTML).join('+').replace(/:/g, '%3A')  to get a complete scope string.

analytics%3Aread%3Aextensions+analytics%3Aread%3Agames+bits%3Aread+channel%3Abot+channel%3Amanage%3Aads+channel%3Aread%3Aads+channel%3Amanage%3Abroadcast+channel%3Aread%3Acharity+channel%3Aedit%3Acommercial+channel%3Aread%3Aeditors+channel%3Amanage%3Aextensions+channel%3Aread%3Agoals+channel%3Aread%3Aguest_star+channel%3Amanage%3Aguest_star+channel%3Aread%3Ahype_train+channel%3Amanage%3Amoderators+channel%3Aread%3Apolls+channel%3Amanage%3Apolls+channel%3Aread%3Apredictions+channel%3Amanage%3Apredictions+channel%3Amanage%3Araids+channel%3Aread%3Aredemptions+channel%3Amanage%3Aredemptions+channel%3Amanage%3Aschedule+channel%3Aread%3Astream_key+channel%3Aread%3Asubscriptions+channel%3Amanage%3Avideos+channel%3Aread%3Avips+channel%3Amanage%3Avips+clips%3Aedit+moderation%3Aread+moderator%3Amanage%3Aannouncements+moderator%3Amanage%3Aautomod+moderator%3Aread%3Aautomod_settings+moderator%3Amanage%3Aautomod_settings+moderator%3Aread%3Abanned_users+moderator%3Amanage%3Abanned_users+moderator%3Aread%3Ablocked_terms+moderator%3Aread%3Achat_messages+moderator%3Amanage%3Ablocked_terms+moderator%3Amanage%3Achat_messages+moderator%3Aread%3Achat_settings+moderator%3Amanage%3Achat_settings+moderator%3Aread%3Achatters+moderator%3Aread%3Afollowers+moderator%3Aread%3Aguest_star+moderator%3Amanage%3Aguest_star+moderator%3Aread%3Amoderators+moderator%3Aread%3Ashield_mode+moderator%3Amanage%3Ashield_mode+moderator%3Aread%3Ashoutouts+moderator%3Amanage%3Ashoutouts+moderator%3Aread%3Asuspicious_users+moderator%3Aread%3Aunban_requests+moderator%3Amanage%3Aunban_requests+moderator%3Aread%3Avips+moderator%3Aread%3Awarnings+moderator%3Amanage%3Awarnings+user%3Abot+user%3Aedit+user%3Aedit%3Abroadcast+user%3Aread%3Ablocked_users+user%3Amanage%3Ablocked_users+user%3Aread%3Abroadcast+user%3Aread%3Achat+user%3Amanage%3Achat_color+user%3Aread%3Aemail+user%3Aread%3Aemotes+user%3Aread%3Afollows+user%3Aread%3Amoderated_channels+user%3Aread%3Asubscriptions+user%3Aread%3Awhispers+user%3Amanage%3Awhispers+user%3Awrite%3Achat+chat%3Aedit+chat%3Aread+whispers%3Aread
```
