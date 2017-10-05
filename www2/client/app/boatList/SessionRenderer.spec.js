describe('SessionRenderer', function() {
  it('Test basic ops', function() {
    var r = new SessionRenderer();
    for (var i in rawSessions) {
      r.addSession(rawSessions[i]);
    }

    expect(r.renderedArray.get().length).toBe(rawSessions.length);
  });
});

